@echo off
set PATH=c:\Qt\5.7.0_VC14\5.7\msvc2015\bin\;%PATH%  

lupdate ..\..\projects\Qt\SciChart.pro

pause

linguist SciChart_de.ts
