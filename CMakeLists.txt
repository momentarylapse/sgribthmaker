# created by xmake

project(sgribthmaker)
cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)
set(SOURCE 
	src/lib/algebra/crypto.cpp
	src/lib/algebra/vli.cpp
	src/lib/any/any.cpp
	src/lib/base/array.cpp
	src/lib/base/pointer.cpp
	src/lib/base/strings.cpp
	src/lib/file/file.cpp
	src/lib/file/file_op.cpp
	src/lib/file/msg.cpp
	src/lib/file/path.cpp
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
	src/lib/hui/Controls/ControlRevealerGtk.cpp
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
	src/lib/hui/Config.cpp
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
	src/lib/hui/Timer.cpp
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
	src/lib/kaba/compiler/compiler.cpp
	src/lib/kaba/compiler/mapper.cpp
	src/lib/kaba/compiler/serializer.cpp
	src/lib/kaba/compiler/SerializerX.cpp
	src/lib/kaba/compiler/SerialNode.cpp
	src/lib/kaba/dynamic/call.cpp
	src/lib/kaba/dynamic/dynamic.cpp
	src/lib/kaba/dynamic/exception.cpp
	src/lib/kaba/lib/dict.cpp
	src/lib/kaba/lib/extern.cpp
	src/lib/kaba/lib/lib.cpp
	src/lib/kaba/lib/lib_base.cpp
	src/lib/kaba/lib/lib_doc.cpp
	src/lib/kaba/lib/lib_hui.cpp
	src/lib/kaba/lib/lib_image.cpp
	src/lib/kaba/lib/lib_kaba.cpp
	src/lib/kaba/lib/lib_math.cpp
	src/lib/kaba/lib/lib_net.cpp
	src/lib/kaba/lib/lib_nix.cpp
	src/lib/kaba/lib/lib_os.cpp
	src/lib/kaba/lib/lib_threads.cpp
	src/lib/kaba/lib/lib_time.cpp
	src/lib/kaba/lib/lib_vulkan.cpp
	src/lib/kaba/syntax/Block.cpp
	src/lib/kaba/syntax/Class.cpp
	src/lib/kaba/syntax/Constant.cpp
	src/lib/kaba/syntax/Function.cpp
	src/lib/kaba/syntax/Identifier.cpp
	src/lib/kaba/syntax/implicit.cpp
	src/lib/kaba/syntax/lexical.cpp
	src/lib/kaba/syntax/macros.cpp
	src/lib/kaba/syntax/Node.cpp
	src/lib/kaba/syntax/Operator.cpp
	src/lib/kaba/syntax/Parser.cpp
	src/lib/kaba/syntax/preprocessor.cpp
	src/lib/kaba/syntax/Statement.cpp
	src/lib/kaba/syntax/SyntaxTree.cpp
	src/lib/kaba/syntax/Variable.cpp
	src/lib/kaba/CompilerConfiguration.cpp
	src/lib/kaba/Interpreter.cpp
	src/lib/kaba/kaba.cpp
	src/lib/math/complex.cpp
	src/lib/math/interpolation.cpp
	src/lib/math/math.cpp
	src/lib/math/matrix.cpp
	src/lib/math/matrix3.cpp
	src/lib/math/plane.cpp
	src/lib/math/quaternion.cpp
	src/lib/math/random.cpp
	src/lib/math/ray.cpp
	src/lib/math/rect.cpp
	src/lib/math/vec2.cpp
	src/lib/math/vec4.cpp
	src/lib/math/vector.cpp
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
	src/Parser/BaseParser.cpp
	src/Parser/ParserC.cpp
	src/Parser/ParserHui.cpp
	src/Parser/ParserKaba.cpp
	src/Parser/ParserPython.cpp
	src/Parser/ParserShader.cpp
	src/Parser/ParserText.cpp
	src/AutoComplete.cpp
	src/CommandDialog.cpp
	src/Console.cpp
	src/Document.cpp
	src/HighlightScheme.cpp
	src/History.cpp
	src/SettingsDialog.cpp
	src/SgribthMaker.cpp
	src/SourceView.cpp
)

find_package(PkgConfig REQUIRED)

# gtk3 or gtk4 mandatory
set(GTK3_OR_GTK4 "gtk3" CACHE STRING "Use gtk3 or gtk4?")
pkg_check_modules(GTK3 gtk+-3.0)
if(GTK3_FOUND)
	list(APPEND GTK3_OR_GTK4_FOUND "gtk3")
endif()
pkg_check_modules(GTK4 gtk4)
if(GTK4_FOUND)
	list(APPEND GTK3_OR_GTK4_FOUND "gtk4")
endif()
if(GTK3_OR_GTK4_FOUND)
	if(NOT ${GTK3_OR_GTK4} IN_LIST GTK3_OR_GTK4_FOUND)
		list(GET GTK3_OR_GTK4_FOUND 0 X)
		set(GTK3_OR_GTK4 ${X} CACHE STRING "choose" FORCE)
		message(WARNING "Choice for GTK3_OR_GTK4 invalid, using ${GTK3_OR_GTK4} instead")
	endif()
else()
	message(SEND_ERROR "Neither gtk3 or gtk4 found") #  or/nor..
	set(GTK3_OR_GTK4 "")
endif()
if(GTK3_OR_GTK4 STREQUAL "gtk3")
	include_directories(${GTK3_INCLUDE_DIRS})
	link_directories(${GTK3_LIBRARY_DIRS})
	list(APPEND DEPS ${GTK3_LIBRARIES})
	add_definitions(${GTK3_CFLAGS_OTHER})
	add_compile_definitions(HAS_LIB_GTK3=1)
endif()
if(GTK3_OR_GTK4 STREQUAL "gtk4")
	include_directories(${GTK4_INCLUDE_DIRS})
	link_directories(${GTK4_LIBRARY_DIRS})
	list(APPEND DEPS ${GTK4_LIBRARIES})
	add_definitions(${GTK4_CFLAGS_OTHER})
	add_compile_definitions(HAS_LIB_GTK4=1)
endif()

# zlib mandatory
pkg_check_modules(ZLIB REQUIRED zlib)
include_directories(${ZLIB_INCLUDE_DIRS})
link_directories(${ZLIB_LIBRARY_DIRS})
list(APPEND DEPS ${ZLIB_LIBRARIES})
add_definitions(${ZLIB_CFLAGS_OTHER})
add_compile_definitions(HAS_LIB_ZLIB=1)

# gl mandatory
pkg_check_modules(GL REQUIRED gl)
include_directories(${GL_INCLUDE_DIRS})
link_directories(${GL_LIBRARY_DIRS})
list(APPEND DEPS ${GL_LIBRARIES})
add_definitions(${GL_CFLAGS_OTHER})
add_compile_definitions(HAS_LIB_GL=1)

# xlib (optional)
pkg_check_modules(X11 x11)
if(X11_FOUND)
	include_directories(${X11_INCLUDE_DIRS})
	link_directories(${X11_LIBRARY_DIRS})
	list(APPEND DEPS ${X11_LIBRARIES})
	add_definitions(${X11_CFLAGS_OTHER})
	add_compile_definitions(HAS_LIB_XLIB=1)
else()
	add_compile_definitions(HAS_LIB_XLIB=0)
endif()

# unwind (optional)
pkg_check_modules(LIBUNWIND libunwind)
if(LIBUNWIND_FOUND)
	include_directories(${LIBUNWIND_INCLUDE_DIRS})
	link_directories(${LIBUNWIND_LIBRARY_DIRS})
	list(APPEND DEPS ${LIBUNWIND_LIBRARIES})
	add_definitions(${LIBUNWIND_CFLAGS_OTHER})
	add_compile_definitions(HAS_LIB_UNWIND=1)
else()
	add_compile_definitions(HAS_LIB_UNWIND=0)
endif()

# dl...
list(APPEND DEPS ${CMAKE_DL_LIBS})
add_compile_definitions(HAS_LIB_DL=1)

add_executable(sgribthmaker ${SOURCE})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(sgribthmaker PRIVATE Threads::Threads ${DEPS})

add_compile_definitions(INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

install(TARGETS sgribthmaker DESTINATION bin)
install(DIRECTORY "static/Icon" DESTINATION share/sgribthmaker)
install(FILES "static/hui_resources.txt" DESTINATION share/sgribthmaker)
install(FILES "static/icon.ico" DESTINATION share/sgribthmaker)
install(FILES "static/icon.png" DESTINATION share/sgribthmaker)
install(FILES "static/kaba.ico" DESTINATION share/sgribthmaker)
install(FILES "static/kaba.png" DESTINATION share/sgribthmaker)
install(FILES "static/license.txt" DESTINATION share/sgribthmaker)
install(FILES "static/license_small.txt" DESTINATION share/sgribthmaker)

set(INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
configure_file("static/michisoft-sgribthmaker.desktop" /tmp/michisoft-sgribthmaker.desktop NO_SOURCE_PERMISSIONS)

find_program(XDG_DESKTOP_MENU_EXECUTABLE xdg-desktop-menu)
if(NOT XDG_DESKTOP_MENU_EXECUTABLE STREQUAL "XDG_DESKTOP_MENU_EXECUTABLE_NOTFOUND")
	install(CODE "message(STATUS \"installing desktop file\")")
	install(CODE "execute_process(COMMAND ${XDG_DESKTOP_MENU_EXECUTABLE} install /tmp/michisoft-sgribthmaker.desktop)")
endif()

find_program(XDG_MIME_EXECUTABLE xdg-mime)
if(NOT XDG_MIME_EXECUTABLE STREQUAL "XDG_MIME_EXECUTABLE_NOTFOUND")
	install(CODE "message(STATUS \"installing mime file\")")
	install(CODE "execute_process(COMMAND ${XDG_MIME_EXECUTABLE} install --novendor ${CMAKE_CURRENT_SOURCE_DIR}/static/michisoft-sgribthmaker.xml)")
endif()