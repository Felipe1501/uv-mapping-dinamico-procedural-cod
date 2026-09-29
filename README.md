# UV Mapping Dinâmico e Procedural

Projeto desenvolvido para a disciplina de **Computação Gráfica e Processamento de Imagens** da Universidade Católica de Santos.

Nesta etapa foi implementada a infraestrutura básica de renderização utilizando **Vulkan**, incluindo criação da instância, dispositivo lógico, swapchain, pipeline gráfico, buffers, sincronização e integração dos shaders em GLSL compilados para SPIR-V.

---

## 1. Requisitos

Para compilar e executar o projeto são necessários:

- C++17
- Vulkan SDK
- GLFW
- CMake
- `glslc`

### Ambiente utilizado

- Sistema operacional: macOS
- Arquitetura: ARM64
- Compilador: Apple Clang 21.0.0
- Vulkan: Vulkan SDK 1.4.357
- Driver Vulkan: MoltenVK 1.4.2
- GLFW
- CMake

No macOS, o Vulkan é executado através do **MoltenVK**, utilizando a infraestrutura gráfica do sistema.

---

## 2. Estrutura do projeto

```text
entrega_cod_uv/
├── CMakeLists.txt
├── README.md
│
├── src/
│   ├── main.cpp
│   ├── VulkanApp.hpp
│   └── VulkanApp.cpp
│
├── shaders/
│   ├── uv_mapping.vert
│   ├── uv_mapping.frag
│   ├── uv_mapping.vert.spv
│   └── uv_mapping.frag.spv
│
└── evidencias/
    └── screenshot.png
```

A pasta `build/` é criada durante a compilação e não faz parte do código-fonte necessário para a reprodução do projeto.

---

## 3. Compilação dos shaders

Os shaders são escritos em GLSL e compilados para SPIR-V utilizando o `glslc`.

### Vertex Shader

```bash
glslc shaders/uv_mapping.vert -o shaders/uv_mapping.vert.spv
```

### Fragment Shader

```bash
glslc shaders/uv_mapping.frag -o shaders/uv_mapping.frag.spv
```

Os arquivos `.spv` gerados são carregados pela aplicação Vulkan durante a criação do pipeline gráfico.

---

## 4. Compilação do projeto

Na pasta raiz do projeto:

```bash
cmake -S . -B build
```

Depois:

```bash
cmake --build build -j
```

O executável será criado em:

```text
build/UVMappingEntrega4
```

---

## 5. Execução

Após a compilação:

```bash
./build/UVMappingEntrega4
```

A aplicação abre uma janela e realiza a renderização utilizando Vulkan.

---

## 6. Principais arquivos

### `src/main.cpp`

Responsável por iniciar a aplicação e tratar exceções.

### `src/VulkanApp.hpp`

Contém a declaração da classe `VulkanApp`, estruturas utilizadas pela aplicação e os recursos Vulkan utilizados durante a renderização.

### `src/VulkanApp.cpp`

Contém a implementação da aplicação Vulkan, incluindo:

- criação da instância;
- criação da Surface;
- seleção da GPU;
- criação do dispositivo lógico;
- criação das filas;
- criação da Swapchain;
- criação das Image Views;
- criação do Render Pass;
- criação do Descriptor Set;
- criação do pipeline gráfico;
- criação dos Framebuffers;
- criação dos Command Buffers;
- criação dos buffers;
- sincronização;
- loop principal;
- renderização;
- limpeza dos recursos.

### `shaders/uv_mapping.vert`

Vertex Shader utilizado pelo pipeline gráfico.

### `shaders/uv_mapping.frag`

Fragment Shader responsável pelo cálculo do padrão visual.

### Arquivos `.spv`

Versões compiladas dos shaders utilizadas pelo Vulkan.

---

## 7. Integração dos shaders

Os shaders desenvolvidos anteriormente foram integrados ao pipeline Vulkan desta entrega.

O processo utilizado é:

```text
GLSL
 │
 ├── uv_mapping.vert
 └── uv_mapping.frag
        │
        ▼
      glslc
        │
        ▼
      SPIR-V
        │
        ├── uv_mapping.vert.spv
        └── uv_mapping.frag.spv
        │
        ▼
   VkShaderModule
        │
        ▼
 Graphics Pipeline
```

A aplicação lê os arquivos SPIR-V e cria os respectivos `VkShaderModule`.

Os shaders são associados aos estágios de Vertex Shader e Fragment Shader durante a criação do pipeline gráfico.

---

## 8. Uniform Buffer e tempo

A aplicação utiliza um **Uniform Buffer Object (UBO)** para enviar o valor de tempo para a GPU.

O valor é atualizado durante a execução da aplicação e disponibilizado ao shader através de um Descriptor Set.

O fragment shader utiliza esse valor para alterar o padrão visual ao longo do tempo.

---

## 9. Renderização

A aplicação possui uma pipeline Vulkan completa para realizar a renderização.

O fluxo principal é:

```text
Aplicação
   │
   ▼
Vulkan Instance
   │
   ▼
Physical Device
   │
   ▼
Logical Device
   │
   ▼
Swapchain
   │
   ▼
Render Pass
   │
   ▼
Graphics Pipeline
   │
   ├── Vertex Shader
   │
   └── Fragment Shader
   │
   ▼
Command Buffer
   │
   ▼
Graphics Queue
   │
   ▼
Swapchain
   │
   ▼
Janela
```

Nesta etapa é realizada uma renderização simples para validar o funcionamento da infraestrutura Vulkan.

---

## 10. Funcionalidades implementadas

Nesta entrega estão funcionando:

- inicialização da API Vulkan;
- criação da Vulkan Instance;
- utilização da validation layer;
- seleção da GPU;
- criação do dispositivo lógico;
- criação das filas gráfica e de apresentação;
- criação da Surface;
- criação da Swapchain;
- criação das Image Views;
- criação do Render Pass;
- criação do Framebuffer;
- criação do Graphics Pipeline;
- criação do Command Pool;
- criação dos Command Buffers;
- criação dos Vertex Buffers;
- criação dos Uniform Buffers;
- criação dos Descriptor Sets;
- criação de semáforos e fences;
- carregamento de shaders SPIR-V;
- criação dos Shader Modules;
- execução da renderização na tela;
- atualização do tempo utilizado pelos shaders.

---

## 11. Funcionalidades futuras

A implementação desta etapa tem como objetivo principal estabelecer a infraestrutura Vulkan e integrar os shaders.

Nas próximas etapas poderão ser desenvolvidas e aprimoradas funcionalidades como:

- aplicação do mapeamento UV em geometrias mais complexas;
- utilização de texturas;
- evolução do padrão procedural;
- aplicação do efeito em diferentes objetos;
- melhorias na animação das coordenadas UV;
- aprimoramentos visuais da técnica.

---

## 12. Evidências

A pasta `evidencias/` contém registros da execução da aplicação.

```text
evidencias/
└── resultado.png
```

O resultado demonstra a aplicação em execução e a renderização realizada utilizando Vulkan.