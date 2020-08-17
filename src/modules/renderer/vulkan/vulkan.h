#ifndef RENDERER_VULKAN_H
#define RENDERER_VULKAN_H

#include "../base/base.h"
#include "../../target/base/base.h"
#include <vulkan/vulkan.h>
#include <optional>
#include <array>
#include <vector>

namespace Module::Renderer {

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        inline bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    
    VkVertexInputBindingDescription getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions();

    class Vulkan : public AbstractBase {
    public:
        Vulkan();
        ~Vulkan();

        void setProvider(void *provider) override; 

        void initialize() override;
        void terminate() override;

        void begin() override;
        void end() override;

        void clear() override;

        void test() override;
 
        void renderGraph(float *x, float *y, size_t count) override;
        void renderSpectrogram(float ***spectrogram, size_t *lengths, size_t count) override;

    private:
        void createInstance();
        void setupDebugMessenger();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createSwapChain();
        void createImageViews();
        void createRenderPass();
        void createDescriptorSetLayout();
        void createGraphicsPipeline();
        void createFramebuffers();
        void createCommandPool();
        void createColorResources();
        void createDepthResources();
        void createVertexBuffer();
        void createUniformBuffers();
        void createDescriptorPool();
        void createDescriptorSets();
        void createCommandBuffers();
        void createSyncObjects();
        
        void cleanupSwapChain();
        void recreateSwapChain();

        void updateUniformBuffers();

        VkShaderModule createShaderModule(const std::vector<char>& code);

        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *pBuffer, VkDeviceMemory *pBufferMemory);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        void createImage(uint32_t width, uint32_t height, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage *pImage, VkDeviceMemory *pImageMemory);
        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

        VkSampleCountFlagBits getMaxUsableSampleCount();
        
        VkSurfaceFormatKHR chooseSwapSurfaceFormat();
        VkPresentModeKHR chooseSwapPresentMode();
        VkExtent2D chooseSwapExtent();
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
        int rateDeviceSuitability(VkPhysicalDevice device);
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        bool checkValidationLayerSupport();
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat findDepthFormat();
        bool hasStencilComponent(VkFormat format);

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

        std::vector<const char *> mValidationLayers;
        std::vector<const char *> mDeviceExtensions;
        int mMaxFramesInFlight;

        std::vector<Vertex> mVertices;
        UniformBuffer mUniforms;

        VulkanProvider *mProvider;

        VkInstance mInstance;
        VkDebugUtilsMessengerEXT mDebugMessenger;
        VkSurfaceKHR mSurface;

        VkPhysicalDevice mPhysicalDevice;
        VkDevice mDevice;
 
        VkSampleCountFlagBits mMsCount;

        VkImage mColorImage;
        VkDeviceMemory mColorImageMemory;
        VkImageView mColorImageView;

        VkImage mDepthImage;
        VkDeviceMemory mDepthImageMemory;
        VkImageView mDepthImageView;

        QueueFamilyIndices mIndices;
        VkQueue mGraphicsQueue;
        VkQueue mPresentQueue;

        SwapChainSupportDetails mSwapChainSupport;
        VkSwapchainKHR mSwapChain;
        std::vector<VkImage> mSwapChainImages;
        VkFormat mSwapChainImageFormat;
        VkExtent2D mSwapChainExtent;
        std::vector<VkImageView> mSwapChainImageViews;
        std::vector<VkFramebuffer> mSwapChainFramebuffers;

        VkRenderPass mRenderPass;
        VkDescriptorSetLayout mDescriptorSetLayout;
        VkPipelineLayout mPipelineLayout;
        VkPipeline mGraphicsPipeline;

        VkDescriptorPool mDescriptorPool;
        std::vector<VkDescriptorSet> mDescriptorSets;

        VkCommandPool mCommandPool;
        std::vector<VkCommandBuffer> mCommandBuffers;

        std::vector<VkSemaphore> mImageAvailableSemaphores;
        std::vector<VkSemaphore> mRenderFinishedSemaphores;
        std::vector<VkFence> mInFlightFences;
        std::vector<VkFence> mImagesInFlight;
        int mCurrentFrame;
        uint32_t mImageIndex;

        VkBuffer mVertexBuffer;
        VkDeviceMemory mVertexBufferMemory;
        
        std::vector<VkBuffer> mUniformBuffers;
        std::vector<VkDeviceMemory> mUniformBuffersMemory;
    };

}

#endif // RENDERER_VULKAN_H
