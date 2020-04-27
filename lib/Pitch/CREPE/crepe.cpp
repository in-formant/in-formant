#include "crepe.h"
#include "models.h"
#include <Eigen/Dense>
#include <tensorflow/c/c_api.h>
#include <memory>
#include <iostream>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <vector>
#include <assert.h>
#include <string.h>
#include <fstream>
#include <stdint.h>

static TF_Buffer *read_tf_buffer(int level);

/**
 * A Wrapper for the C API status object.
 */
class CStatus{
public:
    TF_Status *ptr;
    CStatus(){
        ptr = TF_NewStatus();
    }

    /**
     * Dump the current error message.
     */
    void dump_error()const{
        std::cerr << "TF status error: " << TF_Message(ptr) << std::endl;
    }

    /**
     * Return a boolean indicating whether there was a failure condition.
     * @return
     */
    inline bool failure()const{
        return TF_GetCode(ptr) != TF_OK;
    }

    ~CStatus(){
        if(ptr)TF_DeleteStatus(ptr);
    }
};

namespace detail {
    template<class T>
    class TFObjDeallocator;

    template<>
    struct TFObjDeallocator<TF_Status> { static void run(TF_Status *obj) { TF_DeleteStatus(obj); }};

    template<>
    struct TFObjDeallocator<TF_Graph> { static void run(TF_Graph *obj) { TF_DeleteGraph(obj); }};

    template<>
    struct TFObjDeallocator<TF_Tensor> { static void run(TF_Tensor *obj) { TF_DeleteTensor(obj); }};

    template<>
    struct TFObjDeallocator<TF_SessionOptions> { static void run(TF_SessionOptions *obj) { TF_DeleteSessionOptions(obj); }};

    template<>
    struct TFObjDeallocator<TF_Buffer> { static void run(TF_Buffer *obj) { TF_DeleteBuffer(obj); }};

    template<>
    struct TFObjDeallocator<TF_ImportGraphDefOptions> {
        static void run(TF_ImportGraphDefOptions *obj) { TF_DeleteImportGraphDefOptions(obj); }
    };

    template<>
    struct TFObjDeallocator<TF_Session> {
        static void run(TF_Session *obj) {
            CStatus status;
            TF_DeleteSession(obj, status.ptr);
            if (status.failure()) {
                status.dump_error();
            }
        }
    };
}

template<class T> struct TFObjDeleter{
    void operator()(T* ptr) const{
        detail::TFObjDeallocator<T>::run(ptr);
    }
};

template<class T> struct TFObjMeta{
    typedef std::unique_ptr<T, TFObjDeleter<T>> UniquePtr;
};

template<class T> typename TFObjMeta<T>::UniquePtr tf_obj_unique_ptr(T *obj){
    typename TFObjMeta<T>::UniquePtr ptr(obj);
    return ptr;
}

class MySession{
public:
    typename TFObjMeta<TF_Graph>::UniquePtr graph;
    typename TFObjMeta<TF_Session>::UniquePtr session;

    TF_Output inputs, outputs;
};

/**
 * Load a GraphDef from a provided file.
 * @param filename: The file containing the protobuf encoded GraphDef
 * @param input_name: The name of the input placeholder
 * @param output_name: The name of the output tensor
 * @return
 */
MySession *my_model_load(int level, const char *input_name, const char *output_name){
    printf("Loading model, size %d\n", level+1);
    CStatus status;

    auto graph=tf_obj_unique_ptr(TF_NewGraph());
    {
        // Load a protobuf containing a GraphDef
        auto graph_def=tf_obj_unique_ptr(read_tf_buffer(level));
        if(!graph_def){
            return nullptr;
        }

        auto graph_opts=tf_obj_unique_ptr(TF_NewImportGraphDefOptions());
        TF_GraphImportGraphDef(graph.get(), graph_def.get(), graph_opts.get(), status.ptr);
    }

    if(status.failure()){
        status.dump_error();
        return nullptr;
    }

    auto input_op = TF_GraphOperationByName(graph.get(), input_name);
    auto output_op = TF_GraphOperationByName(graph.get(), output_name);
    if(!input_op || !output_op){
        return nullptr;
    }

    auto session = std::make_unique<MySession>();
    {
        auto opts = tf_obj_unique_ptr(TF_NewSessionOptions());
        session->session = tf_obj_unique_ptr(TF_NewSession(graph.get(), opts.get(), status.ptr));
    }

    if(status.failure()){
        return nullptr;
    }
    assert(session);

    graph.swap(session->graph);
    session->inputs = {input_op, 0};
    session->outputs = {output_op, 0};

    return session.release();
}

/**
 * Deallocator for TF_Buffer data.
 * @tparam T
 * @param data
 * @param length
 */
template<class T> static void free_cpp_array(void* data, size_t length){
    delete []((T *)data);
}

/**
 * Deallocator for TF_NewTensor data.
 * @tparam T
 * @param data
 * @param length
 * @param arg
 */
template<class T> static void cpp_array_deallocator(void* data, size_t length, void* arg){
    delete []((T *)data);
}

/**
 * Read the entire content of a file and return it as a TF_Buffer.
 * @param file: The file to be loaded.
 * @return
 */
static TF_Buffer* read_tf_buffer(int level) {
    unsigned long len;
    unsigned char *src;

    switch (level) {
    case MODEL_TINY:
        len = model_tiny_len;
        src = model_tiny;
        break;
    case MODEL_SMALL:
        len = model_small_len;
        src = model_small;
        break;
    case MODEL_MEDIUM:
        len = model_medium_len;
        src = model_medium;
        break;
    case MODEL_LARGE:
        len = model_large_len;
        src = model_large;
        break;
/*    case MODEL_FULL:
        len = model_full_len;
        src = model_full;
        break;*/
    default:
        return nullptr;
    }

    TF_Buffer *buf = TF_NewBuffer();
    buf->data = src;
    buf->length = len;
    buf->data_deallocator = nullptr;
    return buf;
}

#define MY_TENSOR_SHAPE_MAX_DIM 16
struct TensorShape{
    int64_t values[MY_TENSOR_SHAPE_MAX_DIM];
    int dim;

    int64_t size()const{
        assert(dim>=0);
        int64_t v=1;
        for(int i=0;i<dim;i++)v*=values[i];
        return v;
    }
};

/**
 * Convert an ascii string into a tensor. The '0's are mapped to 0 whereas all the others are mapped to 1.
 * @param str: The source string
 * @param shape: The required shape of the tensor, this must be compatible with the size of the input string.
 * @return A tensor with dtype = tf.float32
 */
TF_Tensor *array2tensor(const Eigen::ArrayXd& x, const TensorShape &shape){
    auto size = x.size();
    if(size!=shape.size()){
        //TODO exception
    }

    auto output_array = std::make_unique<float[]>(size);
    {
        float *dst_ptr = output_array.get();
        for(int i = 0; i < size; i++) {
            *dst_ptr = float(x(i));
            dst_ptr++;
        }
    }

    auto output = tf_obj_unique_ptr(TF_NewTensor(TF_FLOAT,
            shape.values, shape.dim,
            (void *)output_array.get(), size*sizeof(float), cpp_array_deallocator<float>, nullptr));
    if(output){
        // The ownership has been successfully transferred
        output_array.release();
    }
    return output.release();
}

using namespace Eigen;

std::unique_ptr<MySession> session(my_model_load(MODEL_SMALL, "input", "k2tfout_0"));

ArrayXd get_activation(const ArrayXd & x) {
    TensorShape input_shape={{1, 1024}, 2};    // python equivalent: input_shape = (1, 28, 28, 1)
    auto input_values = tf_obj_unique_ptr(array2tensor(x, input_shape));
    if(!input_values){
        std::cerr << "Tensor creation failure." << std::endl;
        return ArrayXd::Zero(0);
    }

    CStatus status;
    TF_Tensor* inputs[]={input_values.get()};
    TF_Tensor* outputs[1]={};
    TF_SessionRun(session->session.get(), nullptr,
            &session->inputs, inputs, 1,
            &session->outputs, outputs, 1,
            nullptr, 0, nullptr, status.ptr);
    auto _output_holder = tf_obj_unique_ptr(outputs[0]);

    if(status.failure()){
        status.dump_error();
        return ArrayXd::Zero(0);
    }

    TF_Tensor &output = *outputs[0];
    if(TF_TensorType(&output) != TF_FLOAT){
        std::cerr << "Error, unexpected output tensor type." << std::endl;
        return ArrayXd::Zero(0);
    }

    size_t output_size = TF_TensorByteSize(&output) / sizeof(float);
    auto output_array = (const float *)TF_TensorData(&output);

    ArrayXd y(output_size);
    for (int i = 0; i < signed(output_size); ++i) {
        y(i) = output_array[i];
    }

    return y;
}

double to_local_average_cents(const Eigen::ArrayXd& salience, int center)
{
    static ArrayXd mapping;
    if (mapping.size() != 360) {
        mapping = ArrayXd::LinSpaced(360, 0.0, 7180.0) + 1997.3794084376191;
    }

    int start = std::max<int>(0, center - 4);
    int end = std::min<int>(salience.size() - 1, center + 4);

    auto range = salience(seq(start, end));

    double productSum = (range * mapping(seq(start, end))).sum();
    double weightSum = range.sum();
    
    return productSum / weightSum;
}

double cents_to_hertz(const double cents)
{
    double frequency = 10.0 * pow(2.0, cents / 1200.0);
    
    if (std::isnan(frequency)) {
        return 0.0;
    }
    else {
        return frequency;
    }
}
