#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const
    {
        return graphicsFamily.has_value() &&
               presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

// Dados enviados ao shader.
// O padding mantém o tamanho do bloco compatível com o alinhamento esperado.
struct UniformBufferObject
{
    float time;
    float padding[3];
};

class VulkanApp
{
public:
    void run();

private:
    struct Vertex
    {
        float position[3];
        float normal[3];
        float uv[2];
    };

    static const std::array<Vertex, 3> vertices;

    GLFWwindow* window = nullptr;

    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    VkSwapchainKHR swapChain = VK_NULL_HANDLE;

    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkFormat swapChainImageFormat{};
    VkExtent2D swapChainExtent{};

    VkRenderPass renderPass = VK_NULL_HANDLE;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    std::vector<VkDescriptorSet> descriptorSets;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkCommandPool commandPool = VK_NULL_HANDLE;

    std::vector<VkCommandBuffer> commandBuffers;

    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    std::vector<VkFence> imagesInFlight;

    size_t currentFrame = 0;

private:
    void initWindow();
    void initVulkan();

    void createInstance();
    void createSurface();

    QueueFamilyIndices findQueueFamilies(
        VkPhysicalDevice deviceToCheck
    );

    bool checkDeviceExtensionSupport(
        VkPhysicalDevice deviceToCheck
    );

    SwapChainSupportDetails querySwapChainSupport(
        VkPhysicalDevice deviceToCheck
    );

    bool isDeviceSuitable(
        VkPhysicalDevice deviceToCheck
    );

    void pickPhysicalDevice();
    void createLogicalDevice();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& formats
    );

    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& modes
    );

    VkExtent2D chooseSwapExtent(
        const VkSurfaceCapabilitiesKHR& capabilities
    );

    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createDescriptorSetLayout();

    VkShaderModule createShaderModule(
        const std::vector<char>& code
    );

    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();

    uint32_t findMemoryType(
        uint32_t typeFilter,
        VkMemoryPropertyFlags properties
    );

    void createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& memory
    );

    void createVertexBuffer();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();

    void updateUniformBuffer(
        uint32_t imageIndex
    );

    void createCommandBuffers();
    void createSyncObjects();

    void drawFrame();
    void mainLoop();
    void cleanup();
};