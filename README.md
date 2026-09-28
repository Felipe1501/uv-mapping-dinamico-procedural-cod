# UV Mapping Dinâmico e Procedural

Implementação em C++ utilizando Vulkan para demonstração de técnicas de UV Mapping dinâmico e geração procedural de padrões em tempo real.

Projeto desenvolvido para a disciplina de **Computação Gráfica e Processamento de Imagens** da Universidade Católica de Santos.

---

## Sobre o projeto

O projeto apresenta uma implementação de uma pipeline gráfica utilizando a API Vulkan, com foco na aplicação de coordenadas UV de forma dinâmica e na geração de padrões procedurais diretamente nos shaders.

A movimentação das coordenadas UV é controlada por um parâmetro de tempo enviado à GPU por meio de um Uniform Buffer Object (UBO). O fragment shader utiliza funções matemáticas para gerar o padrão visual e atualizar sua aparência em tempo real.

A aplicação abre uma janela utilizando GLFW e realiza o processo completo de renderização através do Vulkan.

---

## Funcionalidades

- Inicialização da API Vulkan
- Criação de instância Vulkan
- Seleção da GPU
- Criação do dispositivo lógico
- Criação das filas de processamento
- Criação da Surface
- Swapchain
- Image Views
- Render Pass
- Framebuffers
- Graphics Pipeline
- Command Pool e Command Buffers
- Uniform Buffer Object (UBO)
- Descriptor Sets
- Sincronização com semáforos e fences
- Vertex Shader e Fragment Shader em GLSL
- Compilação dos shaders para SPIR-V
- UV Mapping
- Deslocamento dinâmico das coordenadas UV
- Geração procedural de padrões
- Animação em tempo real

---

## Tecnologias utilizadas

- C++17
- Vulkan
- GLSL
- SPIR-V
- GLFW
- CMake
- MoltenVK / Vulkan SDK

---

## Estrutura do projeto

```text
.
├── CMakeLists.txt
├── README.md
├── src/
│   └── main.cpp
│
├── shaders/
│   ├── uv_mapping.vert
│   ├── uv_mapping.vert.spv
│   ├── uv_mapping.frag
│   └── uv_mapping.frag.spv
│
└── build/