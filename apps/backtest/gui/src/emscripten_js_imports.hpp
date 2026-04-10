#ifndef PLUDUX_BACKTEST_GUI_EMSCRIPTEN_JS_IMPORTS_HPP
#define PLUDUX_BACKTEST_GUI_EMSCRIPTEN_JS_IMPORTS_HPP

#ifdef __EMSCRIPTEN__
extern "C" {
void pludux_js_ensure_hidden_file_input(const char* element_id,
                                        const char* accept);

void pludux_js_open_managed_text_file_input(const char* element_id,
                                            void* user_function);

void pludux_js_open_single_text_file(const char* accept,
                                     const void* user_callback,
                                     void* user_data);

void pludux_js_open_multiple_text_files(const char* accept,
                                        const void* user_callback,
                                        void* user_data);

void pludux_js_save_file(const char* file_name,
                         const char* content,
                         const char* mime_type);

int pludux_js_is_file_system_access_supported();

void pludux_js_set_body_cursor(int is_pointer);

void pludux_js_open_url(const char* url);
}
#endif

#endif