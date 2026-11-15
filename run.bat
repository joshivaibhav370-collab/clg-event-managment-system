@echo off
title CampusConnect Server
cd /d "%~dp0"
start "" http://localhost:8080
CampusConnect.exe
pause
