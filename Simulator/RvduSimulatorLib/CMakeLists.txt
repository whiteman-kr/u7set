cmake_minimum_required(VERSION 3.16)

project(RvduSimulatorLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF) # Set to ON if you want gnu11 extensions

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Lua REQUIRED)

if (MSVC)
  # MSVC warning suppressions
  add_compile_options(
	/wd4828 # The file contains a character starting at offset 0x10d2 that is illegal in the current source character set (codepage 65001).
  )
endif (MSVC)

set(TARGET RvduSimulatorLib)

qt_add_library(${TARGET} STATIC
	./Rvdu/alt_errno.h      
	./Rvdu/alt_stdio.h      
	./Rvdu/alt_types.h      
	./Rvdu/defs.h           
	./Rvdu/fat_filelib.h    
	./Rvdu/keycodes.h       
	./Rvdu/rn_ainput.c      
	./Rvdu/rn_ainput.h      
	./Rvdu/rn_cfg.c         
	./Rvdu/rn_cfg.h         
	./Rvdu/rn_desc.c        
	./Rvdu/rn_desc.h        
	./Rvdu/rn_diag.c        
	./Rvdu/rn_diag.h        
	./Rvdu/rn_fofp.c        
	./Rvdu/rn_fofp.h        
	./Rvdu/rn_kbd.h         
	./Rvdu/rn_kbd_sim.c     
	./Rvdu/rn_kbd_sim.h     
	./Rvdu/rn_lapi.c        
	./Rvdu/rn_lapi.h        
	./Rvdu/rn_logview.c     
	./Rvdu/rn_logview.h     
	./Rvdu/rn_msgbox.c      
	./Rvdu/rn_msgbox.h      
	./Rvdu/rn_opto.c        
	./Rvdu/rn_opto.h        
	./Rvdu/rn_pin.c         
	./Rvdu/rn_pin.h         
	./Rvdu/rn_prim.c        
	./Rvdu/rn_prim.h        
	./Rvdu/rn_scfp.c        
	./Rvdu/rn_scfp.h        
	./Rvdu/rn_schema.c      
	./Rvdu/rn_schema.h      
	./Rvdu/rn_sifp.c        
	./Rvdu/rn_sifp.h        
	./Rvdu/rn_signalsview.c 
	./Rvdu/rn_signalsview.h 
	./Rvdu/rn_trendproc.c   
	./Rvdu/rn_trendproc.h   
	./Rvdu/rn_trendview.c   
	./Rvdu/rn_trendview.h   
	./Rvdu/rn_ts.h          
	./Rvdu/rn_ts_sim.c      
	./Rvdu/rn_ts_sim.h      
	./Rvdu/rn_utils.c       
	./Rvdu/rn_utils.h       
	./Rvdu/rn_utils_sim.c   
	./Rvdu/rn_utils_sim.h       
	./Rvdu/rn_vbuf.h        
	./Rvdu/rn_vbuf_sim.c    
	./Rvdu/rn_vbuf_sim.h    
	./Rvdu/system.h         
	RvduSimulatorLib.c
	RvduSimulatorLib.h
)

target_compile_definitions(${TARGET} PRIVATE RVDU_SIM _CRT_SECURE_NO_WARNINGS)

# Lua library
#
target_include_directories(${TARGET} PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(${TARGET} PRIVATE ${LUA_LIBRARIES})

target_precompile_headers(${TARGET} PRIVATE
)

target_link_libraries(${TARGET} PUBLIC Qt6::Core)
#target_link_libraries(${TARGET} PUBLIC CommonLib Behavior ProtobufEx UtilsLib AppSignalLib HardwareLib OnlineLib ClientLib TrendView SimulatorGrpc)

#target_compile_definitions(${TARGET} PRIVATE SIMULATOR_LIBRARY)

target_include_directories(${TARGET} PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Rvdu)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})


#add_code_coverage_options(${TARGET} CODE_COVERAGE)
