cmake_minimum_required(VERSION 3.16)

project(SerialPortConnectorRadwag VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets SerialPort Charts)

include(FetchContent)
FetchContent_Declare(
    QXlsx
    GIT_REPOSITORY https://github.com/QtExcel/QXlsx.git
    GIT_TAG        v1.5.0
    SOURCE_SUBDIR  QXlsx
)
FetchContent_MakeAvailable(QXlsx)

set(PROJECT_SOURCES
        main.cpp
        main_window.cpp
        main_window.h
        main_window.ui
        resources.qrc
        settings/device_form.ui
        settings/device_form.cpp settings/device_form.h
        settings/device.cpp settings/device.h
        settings/devices_list_model.cpp settings/devices_list_model.h
        settings/devices_list_controler.h settings/devices_list_controler.cpp
        message_utils.h message_utils.cpp
        settings/devices_storage.cpp settings/devices_storage.h
        settings/device_connector.cpp settings/device_connector.h
        serialPort/serial_port.cpp serialPort/serial_port.h
        settings/device_data.h settings/device_data.cpp

        radwag/radwag_measure.h radwag/radwag_measure.cpp


        fluid_tabels/fluid_tables_form.h fluid_tabels/fluid_tables_form.cpp fluid_tabels/fluid_tables_form.ui
        fluid_tabels/fluid.h
        fluid_tabels/fluid.cpp
        fluid_tabels/fluid_manager.h fluid_tabels/fluid_manager.cpp
        material_tabels/material_tabels_dialog.h material_tabels/material_tabels_dialog.cpp material_tabels/material_tabels_dialog.ui
        material_tabels/material.h material_tabels/material.cpp
        material_tabels/material_manager.h material_tabels/material_manager.cpp
        tooltip/tooltip_manager.h tooltip/tooltip_manager.cpp
        sample/sample.h sample/sample.cpp
        sample/sample_manager.h sample/sample_manager.cpp
        sample/sample_dialog.h sample/sample_dialog.cpp sample/sample_dialog.ui
        radwag/measurement.h radwag/measurement.cpp
        radwag/measurement_results.h radwag/measurement_results.cpp
        radwag/measurement_manager.h radwag/measurement_manager.cpp
        main_widow/measurement_controller.h main_widow/measurement_controller.cpp
        utils.h utils.cpp
        schema_img/schema_density_solids.png
        radwag/radwag_scale_connector.h radwag/radwag_scale_connector.cpp
        radwag/view/measurement_tree_model.h radwag/view/measurement_tree_model.cpp
        radwag/view/measurement_sort_filter_proxy_model.h radwag/view/measurement_sort_filter_proxy_model.cpp
        charts/porosity_chart.h charts/porosity_chart.cpp
        charts/base_chart_widget.h charts/base_chart_widget.cpp
        charts/denisty_chart.h charts/denisty_chart.cpp

        sieveAnalysis/sieve_analysis_study.h sieveAnalysis/sieve_analysis_study.cpp
        sieveAnalysis/sieve_analysis_stages.h sieveAnalysis/sieve_analysis_stages.cpp
        main_window_ui_handlers/hydrostatic_measure_module/hydrostatic_measure_module.h main_window_ui_handlers/hydrostatic_measure_module/hydrostatic_measure_module.cpp
        main_window_ui_handlers/hydrostatic_measure_module/measurement_process_ui_handler.h main_window_ui_handlers/hydrostatic_measure_module/measurement_process_ui_handler.cpp
        main_window_ui_handlers/devices_manager_module/devices_manager_ui_handler.h main_window_ui_handlers/devices_manager_module/devices_manager_ui_handler.cpp
        main_window_ui_handlers/hydrostatic_measure_module/measurement_library_ui_handler.h main_window_ui_handlers/hydrostatic_measure_module/measurement_library_ui_handler.cpp
        main_window_ui_handlers/hydrostatic_measure_module/data_holder/hydrostatic_data_holder.h main_window_ui_handlers/hydrostatic_measure_module/data_holder/hydrostatic_data_holder.cpp
        app_core.h app_core.cpp
        main_window_ui_handlers/sieve_analysis_module/sieve_analysis_module.h main_window_ui_handlers/sieve_analysis_module/sieve_analysis_module.cpp
        main_window_ui_handlers/sieve_analysis_module/sieve_analysis_process_ui_handler.h main_window_ui_handlers/sieve_analysis_module/sieve_analysis_process_ui_handler.cpp
        state_machine.h state_machine.cpp
        main_window_ui_handlers/hydrostatic_measure_module/measurement_state_machine.h main_window_ui_handlers/hydrostatic_measure_module/measurement_state_machine.cpp
        main_window_ui_handlers/hydrostatic_measure_module/library_dialogs/summary_measure_dialog.h main_window_ui_handlers/hydrostatic_measure_module/library_dialogs/summary_measure_dialog.cpp main_window_ui_handlers/hydrostatic_measure_module/library_dialogs/summary_measure_dialog.ui
        main_window_ui_handlers/hydrostatic_measure_module/library_dialogs/analysis_measures_dialog.h main_window_ui_handlers/hydrostatic_measure_module/library_dialogs/analysis_measures_dialog.cpp main_window_ui_handlers/hydrostatic_measure_module/library_dialogs/analysis_measures_dialog.ui
        radwag/measure_statistic_analyzer.h radwag/measure_statistic_analyzer.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SerialPortConnectorRadwag
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        excel/export_excel_base.h excel/export_excel_base.cpp

        excel/measurement_exporter.h excel/measurement_exporter.cpp
        excel/section_excel.h excel/section.cpp
        excel/style_formatter.h excel/style_formatter.cpp





    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET SerialPortConnectorRadwag APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(SerialPortConnectorRadwag SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(SerialPortConnectorRadwag
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(SerialPortConnectorRadwag PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::SerialPort
    Qt${QT_VERSION_MAJOR}::Charts
    QXlsx::QXlsx
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SerialPortConnectorRadwag)
endif()
set_target_properties(SerialPortConnectorRadwag PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS SerialPortConnectorRadwag
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SerialPortConnectorRadwag)
endif()
