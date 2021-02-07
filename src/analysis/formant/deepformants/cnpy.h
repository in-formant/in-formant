//Copyright (C) 2011  Carl Rogers
//Released under MIT License
//license available in LICENSE file, or at http://www.opensource.org/licenses/mit-license.php

#ifndef LIBCNPY_H_
#define LIBCNPY_H_

#include "rpcxx.h"
#include<string>
#include<stdexcept>
#include<sstream>
#include<cstdio>
#include<typeinfo>
#include<iostream>
#include<cassert>
#include<memory>
#include<cstdint>
#include<numeric>
#include <QFile>

namespace cnpy {

    struct NpyArray {
        NpyArray(const rpm::vector<size_t>& _shape, size_t _word_size, bool _fortran_order) :
            shape(_shape), word_size(_word_size), fortran_order(_fortran_order)
        {
            num_vals = 1;
            for(size_t i = 0;i < shape.size();i++) num_vals *= shape[i];
            data_holder = std::shared_ptr<rpm::vector<char>>(
                new rpm::vector<char>(num_vals * word_size));
        }

        NpyArray() : shape(0), word_size(0), fortran_order(0), num_vals(0) { }

        template<typename T>
        T* data() {
            return reinterpret_cast<T*>(&(*data_holder)[0]);
        }

        template<typename T>
        const T* data() const {
            return reinterpret_cast<T*>(&(*data_holder)[0]);
        }

        template<typename T>
        rpm::vector<T> as_vec() const {
            const T* p = data<T>();
            return rpm::vector<T>(p, p+num_vals);
        }

        size_t num_bytes() const {
            return data_holder->size();
        }

        std::shared_ptr<rpm::vector<char>> data_holder;
        rpm::vector<size_t> shape;
        size_t word_size;
        bool fortran_order;
        size_t num_vals;
    };
   
    using npz_t = rpm::map<std::string, NpyArray>; 

    char BigEndianTest();
    char map_type(const std::type_info& t);
    template<typename T> rpm::vector<char> create_npy_header(const rpm::vector<size_t>& shape);
    void parse_npy_header(QFile& ds,size_t& word_size, rpm::vector<size_t>& shape, bool& fortran_order);
    void parse_npy_header(unsigned char* buffer,size_t& word_size, rpm::vector<size_t>& shape, bool& fortran_order);
    void parse_zip_footer(QFile& ds, uint16_t& nrecs, size_t& global_header_size, size_t& global_header_offset);
    npz_t npz_load(QFile& ds);
    NpyArray npz_load(QFile& ds, std::string varname);
    NpyArray npy_load(QFile& ds);

    template<typename T> rpm::vector<char>& operator+=(rpm::vector<char>& lhs, const T rhs) {
        //write in little endian
        for(size_t byte = 0; byte < sizeof(T); byte++) {
            char val = *((char*)&rhs+byte); 
            lhs.push_back(val);
        }
        return lhs;
    }

}

#endif
