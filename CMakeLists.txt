# created by xmake

cmake_minimum_required(VERSION 3.21)
project(sgribthmaker)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)
set(SOURCE
	src/dialog/CommandDialog.cpp
	src/dialog/SettingsDialog.cpp
	src/lib/algebra/crypto.cpp
	src/lib/algebra/vli.cpp
	src/lib/any/any.cpp
	src/lib/base/array.cpp
	src/lib/base/pointer.cpp
	src/lib/base/strings.cpp
	src/lib/doc/chunked.cpp
	src/lib/doc/pdf.cpp
	src/lib/doc/ttf.cpp
	src/lib/doc/xml.cpp
	src/lib/hui/Controls/Control.cpp
	src/lib/hui/Controls/ControlButtonGtk.cpp
	src/lib/hui/Controls/ControlCheckBoxGtk.cpp
	src/lib/hui/Controls/ControlColorButtonGtk.cpp
	src/lib/hui/Controls/ControlComboBoxGtk.cpp
	src/lib/hui/Controls/ControlDrawingAreaGtk.cpp
	src/lib/hui/Controls/ControlEditGtk.cpp
	src/lib/hui/Controls/ControlExpanderGtk.cpp
	src/lib/hui/Controls/ControlGridGtk.cpp
	src/lib/hui/Controls/ControlGroupGtk.cpp
	src/lib/hui/Controls/ControlHeaderBar.cpp
	src/lib/hui/Controls/ControlLabelGtk.cpp
	src/lib/hui/Controls/ControlListViewGtk.cpp
	src/lib/hui/Controls/ControlMenuButtonGtk.cpp
	src/lib/hui/Controls/ControlMultilineEditGtk.cpp
	src/lib/hui/Controls/ControlPanedGtk.cpp
	src/lib/hui/Controls/ControlProgressBarGtk.cpp
	src/lib/hui/Controls/ControlRadioButtonGtk.cpp
	src/lib/hui/Controls/ControlScrollerGtk.cpp
	src/lib/hui/Controls/ControlSeparatorGtk.cpp
	src/lib/hui/Controls/ControlSliderGtk.cpp
	src/lib/hui/Controls/ControlSpinButtonGtk.cpp
	src/lib/hui/Controls/ControlTabControlGtk.cpp
	src/lib/hui/Controls/ControlToggleButtonGtk.cpp
	src/lib/hui/Controls/ControlTreeViewGtk.cpp
	src/lib/hui/Controls/MenuItemGtk.cpp
	src/lib/hui/Controls/MenuItemSeparatorGtk.cpp
	src/lib/hui/Controls/MenuItemSeparatorWin.cpp
	src/lib/hui/Controls/MenuItemSubmenuGtk.cpp
	src/lib/hui/Controls/MenuItemSubmenuWin.cpp
	src/lib/hui/Controls/MenuItemToggleGtk.cpp
	src/lib/hui/Controls/MenuItemToggleWin.cpp
	src/lib/hui/Controls/MenuItemWin.cpp
	src/lib/hui/Controls/ToolItemButtonGtk.cpp
	src/lib/hui/Controls/ToolItemButtonWin.cpp
	src/lib/hui/Controls/ToolItemMenuButtonGtk.cpp
	src/lib/hui/Controls/ToolItemMenuButtonWin.cpp
	src/lib/hui/Controls/ToolItemSeparatorGtk.cpp
	src/lib/hui/Controls/ToolItemSeparatorWin.cpp
	src/lib/hui/Controls/ToolItemToggleButtonGtk.cpp
	src/lib/hui/Controls/ToolItemToggleButtonWin.cpp
	src/lib/hui/Application.cpp
	src/lib/hui/Callback.cpp
	src/lib/hui/Clipboard.cpp
	src/lib/hui/common_dlg.cpp
	src/lib/hui/common_dlg_gtk.cpp
	src/lib/hui/common_dlg_win.cpp
	src/lib/hui/error.cpp
	src/lib/hui/Event.cpp
	src/lib/hui/hui.cpp
	src/lib/hui/language.cpp
	src/lib/hui/main_gtk.cpp
	src/lib/hui/main_win.cpp
	src/lib/hui/Menu.cpp
	src/lib/hui/MenuGtk.cpp
	src/lib/hui/MenuWin.cpp
	src/lib/hui/PainterGtk.cpp
	src/lib/hui/Panel.cpp
	src/lib/hui/Resource.cpp
	src/lib/hui/Toolbar.cpp
	src/lib/hui/ToolbarGtk.cpp
	src/lib/hui/ToolbarWin.cpp
	src/lib/hui/utility.cpp
	src/lib/hui/Window.cpp
	src/lib/hui/window_control.cpp
	src/lib/hui/window_control_gtk.cpp
	src/lib/hui/window_control_win.cpp
	src/lib/hui/WindowGtk.cpp
	src/lib/hui/WindowWin.cpp
	src/lib/image/color.cpp
	src/lib/image/image.cpp
	src/lib/image/image_bmp.cpp
	src/lib/image/image_jpg.cpp
	src/lib/image/image_png.cpp
	src/lib/image/image_tga.cpp
	src/lib/image/ImagePainter.cpp
	src/lib/kaba/asm/asm.cpp
	src/lib/kaba/asm/asm_arm.cpp
	src/lib/kaba/asm/asm_x86.cpp
	src/lib/kaba/compiler/Backend.cpp
	src/lib/kaba/compiler/BackendAmd64.cpp
	src/lib/kaba/compiler/BackendARM.cpp
	src/lib/kaba/compiler/BackendX86.cpp
	src/lib/kaba/compiler/CommandList.cpp
	src/lib/kaba/compiler/Compiler.cpp
	src/lib/kaba/compiler/mapper.cpp
	src/lib/kaba/compiler/Serializer.cpp
	src/lib/kaba/compiler/SerialNode.cpp
	src/lib/kaba/dynamic/call.cpp
	src/lib/kaba/dynamic/dynamic.cpp
	src/lib/kaba/dynamic/exception.cpp
	src/lib/kaba/dynamic/sorting.cpp
	src/lib/kaba/lib/dict.cpp
	src/lib/kaba/lib/extern.cpp
	src/lib/kaba/lib/lib.cpp
	src/lib/kaba/lib/lib_base.cpp
	src/lib/kaba/lib/lib_doc.cpp
	src/lib/kaba/lib/lib_gl.cpp
	src/lib/kaba/lib/lib_hui.cpp
	src/lib/kaba/lib/lib_image.cpp
	src/lib/kaba/lib/lib_kaba.cpp
	src/lib/kaba/lib/lib_math.cpp
	src/lib/kaba/lib/lib_net.cpp
	src/lib/kaba/lib/lib_os.cpp
	src/lib/kaba/lib/lib_threads.cpp
	src/lib/kaba/lib/lib_time.cpp
	src/lib/kaba/lib/lib_vulkan.cpp
	src/lib/kaba/lib/list.cpp
	src/lib/kaba/parser/abstract.cpp
	src/lib/kaba/parser/Concretifier.cpp
	src/lib/kaba/parser/import.cpp
	src/lib/kaba/parser/lexical.cpp
	src/lib/kaba/parser/macros.cpp
	src/lib/kaba/parser/Parser.cpp
	src/lib/kaba/parser/Transformer.cpp
	src/lib/kaba/parser/type_matching.cpp
	src/lib/kaba/syntax/Block.cpp
	src/lib/kaba/syntax/Class.cpp
	src/lib/kaba/syntax/Constant.cpp
	src/lib/kaba/syntax/Function.cpp
	src/lib/kaba/syntax/Identifier.cpp
	src/lib/kaba/syntax/Node.cpp
	src/lib/kaba/syntax/Operator.cpp
	src/lib/kaba/syntax/preprocessor.cpp
	src/lib/kaba/syntax/Statement.cpp
	src/lib/kaba/syntax/SyntaxTree.cpp
	src/lib/kaba/syntax/Variable.cpp
	src/lib/kaba/template/implicit.cpp
	src/lib/kaba/template/implicit_array.cpp
	src/lib/kaba/template/implicit_callable.cpp
	src/lib/kaba/template/implicit_dict.cpp
	src/lib/kaba/template/implicit_enum.cpp
	src/lib/kaba/template/implicit_future.cpp
	src/lib/kaba/template/implicit_list.cpp
	src/lib/kaba/template/implicit_optional.cpp
	src/lib/kaba/template/implicit_pointer.cpp
	src/lib/kaba/template/implicit_product.cpp
	src/lib/kaba/template/implicit_regular.cpp
	src/lib/kaba/template/template.cpp
	src/lib/kaba/CompilerConfiguration.cpp
	src/lib/kaba/Context.cpp
	src/lib/kaba/Interpreter.cpp
	src/lib/kaba/kaba.cpp
	src/lib/kaba/Module.cpp
	src/lib/math/complex.cpp
	src/lib/math/interpolation.cpp
	src/lib/math/mat3.cpp
	src/lib/math/mat4.cpp
	src/lib/math/math.cpp
	src/lib/math/plane.cpp
	src/lib/math/quaternion.cpp
	src/lib/math/random.cpp
	src/lib/math/ray.cpp
	src/lib/math/rect.cpp
	src/lib/math/vec2.cpp
	src/lib/math/vec3.cpp
	src/lib/math/vec4.cpp
	src/lib/net/BinaryBuffer.cpp
	src/lib/net/net.cpp
	src/lib/net/Socket.cpp
	src/lib/nix/nix.cpp
	src/lib/nix/nix_buffer.cpp
	src/lib/nix/nix_draw.cpp
	src/lib/nix/nix_framebuffer.cpp
	src/lib/nix/nix_light.cpp
	src/lib/nix/nix_shader.cpp
	src/lib/nix/nix_textures.cpp
	src/lib/nix/nix_vertexbuffer.cpp
	src/lib/nix/nix_view.cpp
	src/lib/os/CommandLineParser.cpp
	src/lib/os/config.cpp
	src/lib/os/date.cpp
	src/lib/os/file.cpp
	src/lib/os/filesystem.cpp
	src/lib/os/formatter.cpp
	src/lib/os/msg.cpp
	src/lib/os/path.cpp
	src/lib/os/stream.cpp
	src/lib/os/terminal.cpp
	src/lib/os/time.cpp
	src/lib/pattern/Observable.cpp
	src/parser/BaseParser.cpp
	src/parser/ParserC.cpp
	src/parser/ParserHui.cpp
	src/parser/ParserIni.cpp
	src/parser/ParserKaba.cpp
	src/parser/ParserPython.cpp
	src/parser/ParserShader.cpp
	src/parser/ParserText.cpp
	src/AutoComplete.cpp
	src/CodeCompiler.cpp
	src/Console.cpp
	src/Document.cpp
	src/HighlightScheme.cpp
	src/History.cpp
	src/LineNumberView.cpp
	src/SgribthMaker.cpp
	src/SgribthMakerWindow.cpp
	src/SourceView.cpp
)

add_executable(sgribthmaker ${SOURCE})

find_package(PkgConfig REQUIRED)


# lib: gtk4 or gtk3 (mandatory)
set(GTK4_OR_GTK3 "gtk4" CACHE STRING "Use gtk4 or gtk3?")
pkg_check_modules(GTK4 gtk4)
if(GTK4_FOUND)
	list(APPEND GTK4_OR_GTK3_FOUND "gtk4")
endif()
pkg_check_modules(GTK3 gtk+-3.0)
if(GTK3_FOUND)
	list(APPEND GTK4_OR_GTK3_FOUND "gtk3")
endif()
if(GTK4_OR_GTK3_FOUND)
	if(NOT ${GTK3_OR_GTK4} IN_LIST GTK4_OR_GTK3_FOUND)
		list(GET GTK4_OR_GTK3_FOUND 0 X)
		set(GTK4_OR_GTK3 ${X} CACHE STRING "choose" FORCE)
		message(WARNING "Choice for GTK4_OR_GTK3 invalid, using ${GTK4_OR_GTK3} instead")
	endif()
else()
	message(SEND_ERROR "Neither gtk4 or gtk3 found") #  or/nor..
	set(GTK4_OR_GTK3 "")
endif()
if(GTK4_OR_GTK3 STREQUAL "gtk4")
	target_include_directories(sgribthmaker PUBLIC ${GTK4_INCLUDE_DIRS})
	target_link_directories(sgribthmaker PUBLIC ${GTK4_LIBRARY_DIRS})
	list(APPEND DEPS ${GTK4_LIBRARIES})
	target_compile_options(sgribthmaker PUBLIC ${GTK4_CFLAGS_OTHER})
	target_compile_definitions(sgribthmaker PUBLIC HAS_LIB_GTK4=1)
endif()
if(GTK4_OR_GTK3 STREQUAL "gtk3")
	target_include_directories(sgribthmaker PUBLIC ${GTK3_INCLUDE_DIRS})
	target_link_directories(sgribthmaker PUBLIC ${GTK3_LIBRARY_DIRS})
	list(APPEND DEPS ${GTK3_LIBRARIES})
	target_compile_options(sgribthmaker PUBLIC ${GTK3_CFLAGS_OTHER})
	target_compile_definitions(sgribthmaker PUBLIC HAS_LIB_GTK3=1)
endif()

# lib: adwaita (optional)
set(LIB_ADWAITA_ENABLED true CACHE BOOL "Use adwaita library?")
pkg_check_modules(LIBADWAITA1 libadwaita-1)
if(LIBADWAITA1_FOUND)
	target_include_directories(sgribthmaker PUBLIC ${LIBADWAITA1_INCLUDE_DIRS})
	target_link_directories(sgribthmaker PUBLIC ${LIBADWAITA1_LIBRARY_DIRS})
	list(APPEND DEPS ${LIBADWAITA1_LIBRARIES})
	target_compile_options(sgribthmaker PUBLIC ${LIBADWAITA1_CFLAGS_OTHER})
else()
	set(LIB_ADWAITA_ENABLED false CACHE BOOL "adwaita library not found" FORCE)
endif()
if(${LIB_ADWAITA_ENABLED})
	target_compile_definitions(sgribthmaker PUBLIC HAS_LIB_ADWAITA=1)
else()
	target_compile_definitions(sgribthmaker PUBLIC HAS_LIB_ADWAITA=0)
endif()

# lib: zlib mandatory
pkg_check_modules(ZLIB REQUIRED zlib)
target_include_directories(sgribthmaker PUBLIC ${ZLIB_INCLUDE_DIRS})
target_link_directories(sgribthmaker PUBLIC ${ZLIB_LIBRARY_DIRS})
list(APPEND DEPS ${ZLIB_LIBRARIES})
target_compile_options(sgribthmaker PUBLIC ${ZLIB_CFLAGS_OTHER})
target_compile_definitions(sgribthmaker PUBLIC HAS_LIB_ZLIB=1)

# lib: gl mandatory
pkg_check_modules(GL REQUIRED gl)
target_include_directories(sgribthmaker PUBLIC ${GL_INCLUDE_DIRS})
target_link_directories(sgribthmaker PUBLIC ${GL_LIBRARY_DIRS})
list(APPEND DEPS ${GL_LIBRARIES})
target_compile_options(sgribthmaker PUBLIC ${GL_CFLAGS_OTHER})
target_compile_definitions(sgribthmaker PUBLIC HAS_LIB_GL=1)

# lib: xlib (optional)
set(LIB_XLIB_ENABLED true CACHE BOOL "Use xlib library?")
pkg_check_modules(X11 x11)
if(X11_FOUND)
	target_include_directories(sgribthmaker PUBLIC ${X11_INCLUDE_DIRS})
	target_link_directories(sgribthmaker PUBLIC ${X11_LIBRARY_DIRS})
	list(APPEND DEPS ${X11_LIBRARIES})
	target_compile_options(sgribthmaker PUBLIC ${X11_CFLAGS_OTHER})
else()
	set(LIB_XLIB_ENABLED false CACHE BOOL "xlib library not found" FORCE)
endif()
if(${LIB_XLIB_ENABLED})
	target_compile_definitions(sgribthmaker PUBLIC HAS_LIB_XLIB=1)
else()
	target_compile_definitions(sgribthmaker PUBLIC HAS_LIB_XLIB=0)
endif()

# lib: unwind (optional)
set(LIB_UNWIND_ENABLED true CACHE BOOL "Use unwind library?")
pkg_check_modules(LIBUNWIND libunwind)
if(LIBUNWIND_FOUND)
	target_include_directories(sgribthmaker PUBLIC ${LIBUNWIND_INCLUDE_DIRS})
	target_link_directories(sgribthmaker PUBLIC ${LIBUNWIND_LIBRARY_DIRS})
	list(APPEND DEPS ${LIBUNWIND_LIBRARIES})
	target_compile_options(sgribthmaker PUBLIC ${LIBUNWIND_CFLAGS_OTHER})
else()
	set(LIB_UNWIND_ENABLED false CACHE BOOL "unwind library not found" FORCE)
endif()
if(${LIB_UNWIND_ENABLED})
	target_compile_definitions(sgribthmaker PUBLIC HAS_LIB_UNWIND=1)
else()
	target_compile_definitions(sgribthmaker PUBLIC HAS_LIB_UNWIND=0)
endif()

# lib: dl...
if(WIN32)
	target_compile_definitions(sgribthmaker PUBLIC HAS_LIB_DL=0)
else()
	list(APPEND DEPS ${CMAKE_DL_LIBS})
	target_compile_definitions(sgribthmaker PUBLIC HAS_LIB_DL=1)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
list(APPEND DEPS Threads::Threads)

target_link_libraries(sgribthmaker PRIVATE ${DEPS})

target_compile_definitions(sgribthmaker PUBLIC INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

install(TARGETS sgribthmaker DESTINATION bin)
install(DIRECTORY "static/Icon" DESTINATION share/sgribthmaker)
install(DIRECTORY "static/icons" DESTINATION share/sgribthmaker)
install(FILES "static/hui_resources.txt" DESTINATION share/sgribthmaker)
install(FILES "static/icon.ico" DESTINATION share/sgribthmaker)
install(FILES "static/icon.png" DESTINATION share/sgribthmaker)
install(FILES "static/kaba.ico" DESTINATION share/sgribthmaker)
install(FILES "static/kaba.png" DESTINATION share/sgribthmaker)
install(FILES "static/license.txt" DESTINATION share/sgribthmaker)
install(FILES "static/license_small.txt" DESTINATION share/sgribthmaker)

# install .desktop file
set(INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/static/michisoft-sgribthmaker.desktop" /tmp/michisoft-sgribthmaker.desktop NO_SOURCE_PERMISSIONS)
install(FILES /tmp/michisoft-sgribthmaker.desktop DESTINATION share/applications)

# install mime
find_program(XDG_MIME_EXECUTABLE xdg-mime)
if(NOT XDG_MIME_EXECUTABLE STREQUAL "XDG_MIME_EXECUTABLE_NOTFOUND")
	install(CODE "message(STATUS \"installing mime file\")")
	install(CODE "execute_process(COMMAND ${XDG_MIME_EXECUTABLE} install --novendor ${CMAKE_CURRENT_SOURCE_DIR}/static/michisoft-sgribthmaker.xml)")
endif()
