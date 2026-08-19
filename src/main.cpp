#include "cpu.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <vector>

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "RISC-V GUI Simulator Dashboard", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    RV32ISimulator cpu;
    std::vector<uint8_t> program = {
        0x93, 0x00, 0x20, 0x03, // addi x1, x0, 50
        0x13, 0x01, 0x40, 0x01, // addi x2, x0, 20
        0x33, 0x01, 0x20, 0x02, // mul  x3, x1, x2
        0x23, 0x20, 0x30, 0x00, // sw   x3, 0(x0)
        0x03, 0x22, 0x00, 0x00  // lw   x4, 0(x0)
    };
    cpu.load_binary(program, 0x0);

    bool auto_run = false;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (auto_run) cpu.step();

        // Control Panel
        ImGui::Begin("CPU Control Panel");
        ImGui::Text("Retired Instructions: %lu", cpu.get_instructions_retired());
        ImGui::Text("Program Counter (PC): 0x%08X", cpu.get_pc());
        if (ImGui::Button("Step")) { cpu.step(); }
        ImGui::SameLine();
        if (ImGui::Button(auto_run ? "Pause" : "Run")) { auto_run = !auto_run; }
        ImGui::SameLine();
        if (ImGui::Button("Reset")) { cpu.reset(); auto_run = false; }
        ImGui::End();

        // Registers Window
        ImGui::Begin("Register File");
        if (ImGui::BeginTable("regs", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Register");
            ImGui::TableSetupColumn("Hex Value");
            ImGui::TableHeadersRow();
            for (int i = 0; i < 32; ++i) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("x%d", i);
                ImGui::TableSetColumnIndex(1); ImGui::Text("0x%08X", cpu.get_register(i));
            }
            ImGui::EndTable();
        }
        ImGui::End();

        // Disassembly Window
        ImGui::Begin("Disassembly View");
        const auto& mem = cpu.get_memory();
        for (size_t addr = 0; addr < program.size(); addr += 4) {
            uint32_t inst = static_cast<uint32_t>(mem[addr]) | (static_cast<uint32_t>(mem[addr+1]) << 8) |
                            (static_cast<uint32_t>(mem[addr+2]) << 16) | (static_cast<uint32_t>(mem[addr+3]) << 24);
            std::string disasm = cpu.disassemble(inst, addr);
            if (cpu.get_pc() == addr) {
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "> 0x%03X: 0x%08X  %s", (int)addr, inst, disasm.c_str());
            } else {
                ImGui::Text("  0x%03X: 0x%08X  %s", (int)addr, inst, disasm.c_str());
            }
        }
        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
