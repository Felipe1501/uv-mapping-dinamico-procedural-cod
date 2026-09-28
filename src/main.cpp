#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
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

// Dados enviados ao vertex shader
// O padding deixa o bloco compatível com o alinhamento std140
struct UniformBufferObject
{
    float time;
    float padding[3];
};

struct Vertex
{
    float position[3];
    float normal[3];
    float uv[2];
};

const std::array<Vertex, 3> vertices = {
    Vertex{{-0.8f, -0.8f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    Vertex{{ 0.8f, -0.8f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
    Vertex{{ 0.0f,  0.8f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}}
};

static std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Nao foi possivel abrir: " + filename);
    }

    const size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
    file.close();

    return buffer;
}

class VulkanApp
{
public:
    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
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

    void initWindow()
    {
        if (!glfwInit())
            throw std::runtime_error("Falha ao inicializar GLFW!");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(
            WIDTH,
            HEIGHT,
            "UV Mapping - Entrega 4",
            nullptr,
            nullptr
        );

        if (!window)
            throw std::runtime_error("Falha ao criar janela!");
    }

    void initVulkan()
    {
        createInstance();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandPool();
        createVertexBuffer();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
        createSyncObjects();

        std::cout << "Vulkan inicializado com sucesso!" << std::endl;
    }

    void createInstance()
    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "UV Mapping Dinamico e Procedural";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Entrega 4 Vulkan";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions =
            glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(
            glfwExtensions,
            glfwExtensions + glfwExtensionCount
        );

        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

#ifdef __APPLE__
        extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

        createInfo.enabledExtensionCount =
            static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        createInfo.enabledLayerCount =
            static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
            throw std::runtime_error("Falha ao criar Vulkan Instance!");

        std::cout << "Instance criada." << std::endl;
    }

    void createSurface()
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
            throw std::runtime_error("Falha ao criar Surface!");

        std::cout << "Surface criada." << std::endl;
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice deviceToCheck)
    {
        QueueFamilyIndices indices;

        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(deviceToCheck, &count, nullptr);
        std::vector<VkQueueFamilyProperties> families(count);
        vkGetPhysicalDeviceQueueFamilyProperties(deviceToCheck, &count, families.data());

        for (uint32_t i = 0; i < count; ++i)
        {
            if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphicsFamily = i;

            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(
                deviceToCheck,
                i,
                surface,
                &presentSupport
            );

            if (presentSupport)
                indices.presentFamily = i;

            if (indices.isComplete())
                break;
        }

        return indices;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice deviceToCheck)
    {
        uint32_t count = 0;
        vkEnumerateDeviceExtensionProperties(deviceToCheck, nullptr, &count, nullptr);

        std::vector<VkExtensionProperties> available(count);
        vkEnumerateDeviceExtensionProperties(
            deviceToCheck,
            nullptr,
            &count,
            available.data()
        );

        std::set<std::string> required(
            deviceExtensions.begin(),
            deviceExtensions.end()
        );

        for (const auto& extension : available)
            required.erase(extension.extensionName);

        return required.empty();
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice deviceToCheck)
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            deviceToCheck,
            surface,
            &details.capabilities
        );

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            deviceToCheck,
            surface,
            &formatCount,
            nullptr
        );
        if (formatCount > 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                deviceToCheck,
                surface,
                &formatCount,
                details.formats.data()
            );
        }

        uint32_t presentCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            deviceToCheck,
            surface,
            &presentCount,
            nullptr
        );
        if (presentCount > 0)
        {
            details.presentModes.resize(presentCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                deviceToCheck,
                surface,
                &presentCount,
                details.presentModes.data()
            );
        }

        return details;
    }

    bool isDeviceSuitable(VkPhysicalDevice deviceToCheck)
    {
        const QueueFamilyIndices indices = findQueueFamilies(deviceToCheck);
        const bool extensionsSupported = checkDeviceExtensionSupport(deviceToCheck);

        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            const auto support = querySwapChainSupport(deviceToCheck);
            swapChainAdequate =
                !support.formats.empty() &&
                !support.presentModes.empty();
        }

        return indices.isComplete() &&
               extensionsSupported &&
               swapChainAdequate;
    }

    void pickPhysicalDevice()
    {
        uint32_t count = 0;
        vkEnumeratePhysicalDevices(instance, &count, nullptr);

        if (count == 0)
            throw std::runtime_error("Nenhuma GPU Vulkan encontrada!");

        std::vector<VkPhysicalDevice> devices(count);
        vkEnumeratePhysicalDevices(instance, &count, devices.data());

        for (VkPhysicalDevice candidate : devices)
        {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(candidate, &properties);

            std::cout << "GPU encontrada: " << properties.deviceName << std::endl;

            if (isDeviceSuitable(candidate))
            {
                physicalDevice = candidate;
                std::cout << "GPU selecionada: " << properties.deviceName << std::endl;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE)
            throw std::runtime_error("Nenhuma GPU adequada encontrada!");
    }

    void createLogicalDevice()
    {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueInfos;
        std::set<uint32_t> uniqueFamilies = {
            indices.graphicsFamily.value(),
            indices.presentFamily.value()
        };

        float priority = 1.0f;
        for (uint32_t family : uniqueFamilies)
        {
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = family;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &priority;
            queueInfos.push_back(queueInfo);
        }

        // MoltenVK/Apple pode exigir VK_KHR_portability_subset.
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(
            physicalDevice,
            nullptr,
            &extensionCount,
            nullptr
        );

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(
            physicalDevice,
            nullptr,
            &extensionCount,
            availableExtensions.data()
        );

        bool portabilitySupported = false;
        for (const auto& extension : availableExtensions)
        {
            if (std::strcmp(extension.extensionName, "VK_KHR_portability_subset") == 0)
            {
                portabilitySupported = true;
                break;
            }
        }

        std::vector<const char*> enabledExtensions = deviceExtensions;
        if (portabilitySupported)
            enabledExtensions.push_back("VK_KHR_portability_subset");

        VkPhysicalDeviceFeatures features{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
        createInfo.pQueueCreateInfos = queueInfos.data();
        createInfo.pEnabledFeatures = &features;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
        createInfo.ppEnabledExtensionNames = enabledExtensions.data();
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
            throw std::runtime_error("Falha ao criar Logical Device!");

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

        std::cout << "Logical Device criado." << std::endl;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& formats)
    {
        for (const auto& format : formats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return format;
        }
        return formats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& modes)
    {
        for (const auto& mode : modes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
                return mode;
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
            return capabilities.currentExtent;

        VkExtent2D extent{WIDTH, HEIGHT};
        extent.width = std::max(
            capabilities.minImageExtent.width,
            std::min(capabilities.maxImageExtent.width, extent.width)
        );
        extent.height = std::max(
            capabilities.minImageExtent.height,
            std::min(capabilities.maxImageExtent.height, extent.height)
        );
        return extent;
    }

    void createSwapChain()
    {
        const auto support = querySwapChainSupport(physicalDevice);
        const auto format = chooseSwapSurfaceFormat(support.formats);
        const auto presentMode = chooseSwapPresentMode(support.presentModes);
        const auto extent = chooseSwapExtent(support.capabilities);

        uint32_t imageCount = support.capabilities.minImageCount + 1;
        if (support.capabilities.maxImageCount > 0 &&
            imageCount > support.capabilities.maxImageCount)
            imageCount = support.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = format.format;
        createInfo.imageColorSpace = format.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t families[] = {
            indices.graphicsFamily.value(),
            indices.presentFamily.value()
        };

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = families;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = support.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
            throw std::runtime_error("Falha ao criar Swapchain!");

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = format.format;
        swapChainExtent = extent;

        std::cout << "Swapchain criada." << std::endl;
    }

    void createImageViews()
    {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); ++i)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
                throw std::runtime_error("Falha ao criar Image View!");
        }

        std::cout << "Image Views criadas." << std::endl;
    }

    void createRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorRef{};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = &colorAttachment;
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpass;
        createInfo.dependencyCount = 1;
        createInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &createInfo, nullptr, &renderPass) != VK_SUCCESS)
            throw std::runtime_error("Falha ao criar Render Pass!");

        std::cout << "Render Pass criado." << std::endl;
    }

    void createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.bindingCount = 1;
        createInfo.pBindings = &binding;

        if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
            throw std::runtime_error("Falha ao criar Descriptor Set Layout!");

        std::cout << "Descriptor Set Layout criado." << std::endl;
    }

    VkShaderModule createShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule = VK_NULL_HANDLE;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
            throw std::runtime_error("Falha ao criar Shader Module!");

        return shaderModule;
    }

    void createGraphicsPipeline()
    {
        const auto vertCode = readFile("shaders/uv_mapping.vert.spv");
        const auto fragCode = readFile("shaders/uv_mapping.frag.spv");

        VkShaderModule vertModule = createShaderModule(vertCode);
        VkShaderModule fragModule = createShaderModule(fragCode);

        VkPipelineShaderStageCreateInfo vertStage{};
        vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertStage.module = vertModule;
        vertStage.pName = "main";

        VkPipelineShaderStageCreateInfo fragStage{};
        fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragStage.module = fragModule;
        fragStage.pName = "main";

        VkPipelineShaderStageCreateInfo stages[] = {vertStage, fragStage};

        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.stride = sizeof(Vertex);
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::array<VkVertexInputAttributeDescription, 3> attributes{};

        attributes[0].binding = 0;
        attributes[0].location = 0;
        attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributes[0].offset = offsetof(Vertex, position);

        attributes[1].binding = 0;
        attributes[1].location = 1;
        attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributes[1].offset = offsetof(Vertex, normal);

        attributes[2].binding = 0;
        attributes[2].location = 2;
        attributes[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributes[2].offset = offsetof(Vertex, uv);

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &binding;
        vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
        vertexInput.pVertexAttributeDescriptions = attributes.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.width = static_cast<float>(swapChainExtent.width);
        viewport.height = static_cast<float>(swapChainExtent.height);
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.extent = swapChainExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &descriptorSetLayout;

        if (vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Falha ao criar Pipeline Layout!");

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = stages;
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;

        if (vkCreateGraphicsPipelines(
                device,
                VK_NULL_HANDLE,
                1,
                &pipelineInfo,
                nullptr,
                &graphicsPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("Falha ao criar Graphics Pipeline!");
        }

        vkDestroyShaderModule(device, fragModule, nullptr);
        vkDestroyShaderModule(device, vertModule, nullptr);

        std::cout << "Graphics Pipeline criada." << std::endl;
    }

    void createFramebuffers()
    {
        swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); ++i)
        {
            VkImageView attachments[] = {swapChainImageViews[i]};

            VkFramebufferCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            info.renderPass = renderPass;
            info.attachmentCount = 1;
            info.pAttachments = attachments;
            info.width = swapChainExtent.width;
            info.height = swapChainExtent.height;
            info.layers = 1;

            if (vkCreateFramebuffer(device, &info, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("Falha ao criar Framebuffer!");
        }

        std::cout << "Framebuffers criados." << std::endl;
    }

    void createCommandPool()
    {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.queueFamilyIndex = indices.graphicsFamily.value();

        if (vkCreateCommandPool(device, &info, nullptr, &commandPool) != VK_SUCCESS)
            throw std::runtime_error("Falha ao criar Command Pool!");

        std::cout << "Command Pool criada." << std::endl;
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties{};
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1u << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        }

        throw std::runtime_error("Tipo de memoria adequado nao encontrado!");
    }

    void createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& memory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
            throw std::runtime_error("Falha ao criar Buffer!");

        VkMemoryRequirements requirements{};
        vkGetBufferMemoryRequirements(device, buffer, &requirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = requirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(requirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
            throw std::runtime_error("Falha ao alocar memoria do Buffer!");

        vkBindBufferMemory(device, buffer, memory, 0);
    }

    void createVertexBuffer()
    {
        const VkDeviceSize size = sizeof(vertices);

        createBuffer(
            size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertexBuffer,
            vertexBufferMemory
        );

        void* data = nullptr;
        vkMapMemory(device, vertexBufferMemory, 0, size, 0, &data);
        std::memcpy(data, vertices.data(), static_cast<size_t>(size));
        vkUnmapMemory(device, vertexBufferMemory);

        std::cout << "Vertex Buffer criado." << std::endl;
    }

    void createUniformBuffers()
    {
        const VkDeviceSize size = sizeof(UniformBufferObject);

        uniformBuffers.resize(swapChainImages.size());
        uniformBuffersMemory.resize(swapChainImages.size());
        uniformBuffersMapped.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); ++i)
        {
            createBuffer(
                size,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                uniformBuffers[i],
                uniformBuffersMemory[i]
            );

            vkMapMemory(
                device,
                uniformBuffersMemory[i],
                0,
                size,
                0,
                &uniformBuffersMapped[i]
            );
        }

        std::cout << "Uniform Buffers criados." << std::endl;
    }

    void createDescriptorPool()
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(swapChainImages.size());

        VkDescriptorPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        info.poolSizeCount = 1;
        info.pPoolSizes = &poolSize;
        info.maxSets = static_cast<uint32_t>(swapChainImages.size());

        if (vkCreateDescriptorPool(device, &info, nullptr, &descriptorPool) != VK_SUCCESS)
            throw std::runtime_error("Falha ao criar Descriptor Pool!");

        std::cout << "Descriptor Pool criada." << std::endl;
    }

    void createDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(
            swapChainImages.size(),
            descriptorSetLayout
        );

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(swapChainImages.size());

        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
            throw std::runtime_error("Falha ao alocar Descriptor Sets!");

        for (size_t i = 0; i < swapChainImages.size(); ++i)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = descriptorSets[i];
            write.dstBinding = 0;
            write.dstArrayElement = 0;
            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write.descriptorCount = 1;
            write.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        }

        std::cout << "Descriptor Sets criados." << std::endl;
    }

    void updateUniformBuffer(uint32_t imageIndex)
    {
        static const auto startTime = std::chrono::high_resolution_clock::now();
        const auto currentTime = std::chrono::high_resolution_clock::now();

        const float time = std::chrono::duration<float>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.time = time;

        std::memcpy(
            uniformBuffersMapped[imageIndex],
            &ubo,
            sizeof(ubo)
        );
    }

    void createCommandBuffers()
    {
        commandBuffers.resize(swapChainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
            throw std::runtime_error("Falha ao alocar Command Buffers!");

        for (size_t i = 0; i < commandBuffers.size(); ++i)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
                throw std::runtime_error("Falha ao iniciar Command Buffer!");

            VkClearValue clearColor{};
            clearColor.color = {{0.04f, 0.04f, 0.10f, 1.0f}};

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = swapChainFramebuffers[i];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = swapChainExtent;
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(
                commandBuffers[i],
                &renderPassInfo,
                VK_SUBPASS_CONTENTS_INLINE
            );

            vkCmdBindPipeline(
                commandBuffers[i],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                graphicsPipeline
            );

            VkBuffer buffers[] = {vertexBuffer};
            VkDeviceSize offsets[] = {0};

            vkCmdBindVertexBuffers(
                commandBuffers[i],
                0,
                1,
                buffers,
                offsets
            );

            vkCmdBindDescriptorSets(
                commandBuffers[i],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                0,
                1,
                &descriptorSets[i],
                0,
                nullptr
            );

            vkCmdDraw(
                commandBuffers[i],
                3,
                1,
                0,
                0
            );

            vkCmdEndRenderPass(commandBuffers[i]);

            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("Falha ao finalizar Command Buffer!");
        }

        std::cout << "Command Buffers gravados." << std::endl;
    }

    void createSyncObjects()
    {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(swapChainImages.size());
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Falha ao criar sincronizacao de frame!");
            }
        }

        for (size_t i = 0; i < renderFinishedSemaphores.size(); ++i)
        {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
                throw std::runtime_error("Falha ao criar renderFinishedSemaphore!");
        }

        std::cout << "Sincronizacao criada." << std::endl;
    }

    void drawFrame()
    {
        vkWaitForFences(
            device,
            1,
            &inFlightFences[currentFrame],
            VK_TRUE,
            UINT64_MAX
        );

        uint32_t imageIndex = 0;
        VkResult result = vkAcquireNextImageKHR(
            device,
            swapChain,
            UINT64_MAX,
            imageAvailableSemaphores[currentFrame],
            VK_NULL_HANDLE,
            &imageIndex
        );

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            throw std::runtime_error("Falha ao adquirir imagem da Swapchain!");

        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
        {
            vkWaitForFences(
                device,
                1,
                &imagesInFlight[imageIndex],
                VK_TRUE,
                UINT64_MAX
            );
        }

        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        // O valor de tempo muda a cada frame e chega ao shader via UBO
        updateUniformBuffer(imageIndex);

        VkSemaphore waitSemaphore = imageAvailableSemaphores[currentFrame];
        VkSemaphore signalSemaphore = renderFinishedSemaphores[imageIndex];
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &waitSemaphore;
        submitInfo.pWaitDstStageMask = &waitStage;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &signalSemaphore;

        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        if (vkQueueSubmit(
                graphicsQueue,
                1,
                &submitInfo,
                inFlightFences[currentFrame]) != VK_SUCCESS)
        {
            throw std::runtime_error("Falha ao enviar comandos para GPU!");
        }

        VkSwapchainKHR swapChains[] = {swapChain};

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &signalSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            throw std::runtime_error("Falha ao apresentar imagem!");

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void mainLoop()
    {
        std::cout << "Entrando no loop de renderizacao..." << std::endl;

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(device);
        std::cout << "Loop de renderizacao encerrado." << std::endl;
    }

    void cleanup()
    {
        if (device != VK_NULL_HANDLE)
            vkDeviceWaitIdle(device);

        for (size_t i = 0; i < imageAvailableSemaphores.size(); ++i)
        {
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        for (VkSemaphore semaphore : renderFinishedSemaphores)
            vkDestroySemaphore(device, semaphore, nullptr);

        for (size_t i = 0; i < uniformBuffers.size(); ++i)
        {
            if (uniformBuffersMapped[i] != nullptr)
                vkUnmapMemory(device, uniformBuffersMemory[i]);

            vkDestroyBuffer(device, uniformBuffers[i], nullptr);
            vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
        }

        if (vertexBuffer != VK_NULL_HANDLE)
            vkDestroyBuffer(device, vertexBuffer, nullptr);

        if (vertexBufferMemory != VK_NULL_HANDLE)
            vkFreeMemory(device, vertexBufferMemory, nullptr);

        if (descriptorPool != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(device, descriptorPool, nullptr);

        for (VkFramebuffer framebuffer : swapChainFramebuffers)
            vkDestroyFramebuffer(device, framebuffer, nullptr);

        if (graphicsPipeline != VK_NULL_HANDLE)
            vkDestroyPipeline(device, graphicsPipeline, nullptr);

        if (pipelineLayout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

        if (descriptorSetLayout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

        if (renderPass != VK_NULL_HANDLE)
            vkDestroyRenderPass(device, renderPass, nullptr);

        for (VkImageView imageView : swapChainImageViews)
            vkDestroyImageView(device, imageView, nullptr);

        if (swapChain != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(device, swapChain, nullptr);

        if (commandPool != VK_NULL_HANDLE)
            vkDestroyCommandPool(device, commandPool, nullptr);

        if (device != VK_NULL_HANDLE)
            vkDestroyDevice(device, nullptr);

        if (surface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(instance, surface, nullptr);

        if (instance != VK_NULL_HANDLE)
            vkDestroyInstance(instance, nullptr);

        if (window != nullptr)
            glfwDestroyWindow(window);

        glfwTerminate();
    }
};

int main()
{
    VulkanApp app;

    try
    {
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERRO: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
