cmake_minimum_required(VERSION 3.21)
project(domino C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
add_compile_options(-Wall -Wextra)

# Regras, CPU, layout e controlador: sem raylib (door 3).
add_library(domino_core STATIC
  src/core/rules.c
  src/core/cpu.c
  src/core/layout.c
  src/core/controller.c
  src/core/cli.c)
target_include_directories(domino_core PUBLIC src/core)

find_package(raylib REQUIRED)

add_library(domino_app STATIC src/ui/app.c src/ui/draw.c)
target_include_directories(domino_app PUBLIC src/ui)
target_link_libraries(domino_app PUBLIC domino_core raylib)

add_executable(domino src/ui/main.c)
target_link_libraries(domino PRIVATE domino_app)
target_link_options(domino PRIVATE -static-libgcc)

# Copia as DLLs da raylib para o executável rodar fora do shell do MSYS2.
get_target_property(raylib_dll raylib IMPORTED_LOCATION_RELEASE)
get_filename_component(raylib_bin "${raylib_dll}" DIRECTORY)
add_custom_command(TARGET domino POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${raylib_dll}" "${raylib_bin}/glfw3.dll" "$<TARGET_FILE_DIR:domino>")

enable_testing()

add_executable(test_core tests/test_core.c)
target_link_libraries(test_core PRIVATE domino_core)

add_executable(test_window tests/test_window.c)
target_link_libraries(test_window PRIVATE domino_app)

add_executable(test_process tests/test_process.c)

foreach(t
    deal_28_unique_7_each deal_same_seed_same_hands cli_parse_seed
    south_hand_sorted_by_sum south_hand_in_bottom_band cpu_rows_match_hand_count
    first_hand_double_six_starts first_hand_rejects_non_double_six
    turn_order_counterclockwise click_single_end_places click_both_ends_waits_choice
    click_equal_ends_goes_right click_invalid_shows_message click_ignored_off_turn
    south_auto_pass cpu_moves_at_800ms cpu_picks_heaviest cpu_pass_message
    cpu_faces_hidden_in_play score_simples score_carroca score_la_e_lo score_cruzada
    score_trancado_lower_sum score_trancado_tie hand_end_overlay next_hand_winner_starts
    next_hand_after_tranque score_text_format match_end_message new_match_resets
    layout_first_tile_centered layout_within_table layout_doubles_perpendicular)
  add_test(NAME ${t} COMMAND test_core ${t})
endforeach()

foreach(t window_table_color window_fail_exits_1)
  add_test(NAME ${t} COMMAND test_window ${t} WORKING_DIRECTORY $<TARGET_FILE_DIR:domino>)
endforeach()

foreach(t cli_bad_args_exit_2 cli_window_title_and_size cli_close_exits_0)
  add_test(NAME ${t} COMMAND test_process ${t} $<TARGET_FILE:domino>)
endforeach()

add_test(NAME core_has_no_raylib
  COMMAND ${CMAKE_COMMAND} -DCORE_DIR=${CMAKE_SOURCE_DIR}/src/core
    -P ${CMAKE_SOURCE_DIR}/tests/core_has_no_raylib.cmake)
