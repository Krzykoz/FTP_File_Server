cmake_minimum_required(VERSION 3.24)
project(FTP_File_Server)

set(CMAKE_CXX_STANDARD 17)

add_executable(FTP_File_Server main.cpp FtpServer.cpp FtpServer.h)
