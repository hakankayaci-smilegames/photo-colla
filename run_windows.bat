@echo off
setlocal enabledelayedexpansion
title PhotoColla - Windows Otomatik Derle ve Calistir

echo ==============================================================================
echo    PhotoColla Studio - Windows Kurulum, Derleme ve Baslatma Scripti
echo ==============================================================================
echo.

set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"

:: -----------------------------------------------------------------------------
:: 1. CMake Kontrolu
:: -----------------------------------------------------------------------------
echo [*] 1/5 - CMake araniyor...
where cmake >nul 2>nul
if %ERRORLEVEL% equ 0 (
    echo [OK] CMake PATH uzerinde bulundu.
) else (
    set "CMAKE_FOUND=0"
    for %%P in (
        "C:\Program Files\CMake\bin\cmake.exe"
        "C:\Program Files (x86)\CMake\bin\cmake.exe"
        "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        "C:\msys64\mingw64\bin\cmake.exe"
    ) do (
        if exist %%P (
            for %%D in (%%P) do set "PATH=%%~dpD;!PATH!"
            set "CMAKE_FOUND=1"
            echo [OK] CMake surada bulundu: %%P
        )
    )
    if "!CMAKE_FOUND!"=="0" (
        echo [!] CMake bulunamadi! winget ile kuruluyor...
        where winget >nul 2>nul
        if %ERRORLEVEL% equ 0 (
            winget install --id Kitware.CMake -e --silent --accept-source-agreements --accept-package-agreements
            set "PATH=C:\Program Files\CMake\bin;!PATH!"
        ) else (
            echo [HATA] winget bulunamadi. Lutfen CMake'i manuel kurun: https://cmake.org/download/
            pause
            exit /b 1
        )
    )
)

:: -----------------------------------------------------------------------------
:: 2. Derleyici Ortami (MSVC vcvars64 veya MinGW)
:: -----------------------------------------------------------------------------
echo.
echo [*] 2/5 - C++20 Derleyici ortami hazirlaniyor...
where cl >nul 2>nul
if %ERRORLEVEL% equ 0 (
    echo [OK] MSVC derleyicisi aktif.
) else (
    where g++ >nul 2>nul
    if %ERRORLEVEL% equ 0 (
        echo [OK] GCC / MinGW derleyicisi bulundu.
        set "GENERATOR_FLAG=-G "MinGW Makefiles""
    ) else (
        set "VCVARS_FOUND=0"
        for %%V in (
            "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
            "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
            "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
            "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
            "C:\Program Files\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
            "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
        ) do (
            if exist %%V (
                echo [OK] Visual Studio ortami yukleniyor: %%V
                call %%V >nul
                set "VCVARS_FOUND=1"
            )
        )
        if "!VCVARS_FOUND!"=="0" (
            if exist "C:\msys64\mingw64\bin\g++.exe" (
                set "PATH=C:\msys64\mingw64\bin;!PATH!"
                set "GENERATOR_FLAG=-G "MinGW Makefiles""
                echo [OK] MSYS2 MinGW derleyicisi aktif edildi.
            ) else (
                echo [!] Derleyici bulunamadi! Visual Studio C++ Build Tools kuruluyor...
                where winget >nul 2>nul
                if %ERRORLEVEL% equ 0 (
                    winget install --id Microsoft.VisualStudio.2022.BuildTools --override "--passive --wait --add Microsoft.VisualStudio.Workload.VCTools"
                    echo [!] Kurulum tamamlandi. Lutfen scripti tekrar calistirin.
                    pause
                    exit /b 0
                ) else (
                    echo [HATA] C++ derleyicisi bulunamadi. Lutfen Visual Studio 2022 (C++ ile) kurun.
                    pause
                    exit /b 1
                )
            )
        )
    )
)

:: -----------------------------------------------------------------------------
:: 3. Qt 6 Kutuphanesi Tespiti ve Konfigurasyonu
:: -----------------------------------------------------------------------------
echo.
echo [*] 3/5 - Qt 6 kutuphaneleri araniyor...
set "QT_DIR="

if defined CMAKE_PREFIX_PATH (
    if exist "%CMAKE_PREFIX_PATH%\lib\cmake\Qt6" set "QT_DIR=%CMAKE_PREFIX_PATH%"
)
if defined QTDIR (
    if exist "%QTDIR%\lib\cmake\Qt6" set "QT_DIR=%QTDIR%"
)

if not defined QT_DIR (
    for /d %%Q in (C:\Qt\6* C:\Qt\Qt6* D:\Qt\6*) do (
        for /d %%M in (%%Q\msvc2022_64 %%Q\mingw_64 %%Q\msvc2019_64) do (
            if exist "%%M\lib\cmake\Qt6" (
                set "QT_DIR=%%M"
            )
        )
    )
)

if not defined QT_DIR (
    if exist "C:\msys64\mingw64\lib\cmake\Qt6" (
        set "QT_DIR=C:\msys64\mingw64"
    )
)

if not defined QT_DIR (
    echo [!] Qt 6 bulunamadi!
    echo [*] Otomatik indirme baslatilabilir (aqtinstall kullanilarak).
    echo [*] Python pip uzerinden Qt 6 kuruluyor...
    where pip >nul 2>nul
    if %ERRORLEVEL% equ 0 (
        pip install aqtinstall
        echo [*] Qt 6 indiriliyor (Bu islem bir defaya mahsus birkac dakika surebilir)...
        python -m aqt install-qt windows desktop 6.8.0 win64_msvc2022_64 -O C:\Qt
        set "QT_DIR=C:\Qt\6.8.0\msvc2022_64"
    ) else (
        echo [HATA] Qt 6 bulunamadi! Lutfen Qt 6'yi kurun: https://www.qt.io/download-qt-installer
        echo veya C:\Qt altina kurup scripti tekrar calistirin.
        pause
        exit /b 1
    )
)

echo [OK] Qt 6 Dizin: %QT_DIR%
set "PATH=%QT_DIR%\bin;!PATH!"

:: -----------------------------------------------------------------------------
:: 4. CMake Ile Derleme (Build)
:: -----------------------------------------------------------------------------
echo.
echo [*] 4/5 - PhotoColla derleniyor (Release)...
set "BUILD_DIR=%SCRIPT_DIR%build_win"

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

cmake -B "%BUILD_DIR%" -S "%SCRIPT_DIR%" ^
    -DCMAKE_PREFIX_PATH="%QT_DIR%" ^
    -DCMAKE_BUILD_TYPE=Release ^
    %GENERATOR_FLAG%

if %ERRORLEVEL% neq 0 (
    echo [HATA] CMake yapilandirmasi basarisiz oldu!
    pause
    exit /b 1
)

cmake --build "%BUILD_DIR%" --config Release -j%NUMBER_OF_PROCESSORS%
if %ERRORLEVEL% neq 0 (
    echo [HATA] Derleme basarisiz oldu!
    pause
    exit /b 1
)

:: -----------------------------------------------------------------------------
:: 5. Windows DLL Paketleme (windeployqt) ve Calistirma
:: -----------------------------------------------------------------------------
echo.
echo [*] 5/5 - Bagimliliklar paketleniyor (windeployqt)...

set "EXE_PATH="
if exist "%BUILD_DIR%\Release\PhotoColla.exe" (
    set "EXE_PATH=%BUILD_DIR%\Release\PhotoColla.exe"
) else if exist "%BUILD_DIR%\PhotoColla.exe" (
    set "EXE_PATH=%BUILD_DIR%\PhotoColla.exe"
)

if not defined EXE_PATH (
    echo [HATA] PhotoColla.exe olusturulamadi!
    pause
    exit /b 1
)

echo [OK] Binary: %EXE_PATH%

:: windeployqt calistirarak tum gerekli Qt DLL ve plugin'lerini kopyala
if exist "%QT_DIR%\bin\windeployqt.exe" (
    "%QT_DIR%\bin\windeployqt.exe" --no-translations --compiler-runtime "%EXE_PATH%" >nul
    echo [OK] Tum Qt DLL ve font bilesenleri paketlendi.
)

:: Calistir
echo.
echo ==============================================================================
echo    [BASARILI] PhotoColla Baslatiliyor...
echo ==============================================================================
start "" "%EXE_PATH%"

exit /b 0
