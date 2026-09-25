#!/usr/bin/env bash
# ==============================================================================
# PhotoColla - Tek Tuşla Derle ve Çalıştır Scripti (Build & Run Script)
# Kullanım:
#   ./run.sh          -> Release profiliyle hızlıca derler ve başlatır
#   ./run.sh --debug  -> Debug profili ve ASan/UBSan ile derler
#   ./run.sh --clean  -> build/ dizinini temizleyip sıfırdan derler
# ==============================================================================

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"
BUILD_TYPE="Release"

# Renkli log çıktıları
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

for arg in "$@"; do
    case "$arg" in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            echo -e "${YELLOW}[!] 'build' klasörü temizleniyor...${NC}"
            rm -rf "${BUILD_DIR}"
            shift
            ;;
    esac
done

echo -e "${BLUE}[*] PhotoColla Yapılandırılıyor (${BUILD_TYPE})...${NC}"
cmake -B "${BUILD_DIR}" -S "${PROJECT_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

echo -e "${BLUE}[*] Derleniyor ($(nproc) thread)...${NC}"
cmake --build "${BUILD_DIR}" -j"$(nproc)"

echo -e "${GREEN}[✓] Derleme başarılı! PhotoColla başlatılıyor...${NC}"
"${BUILD_DIR}/PhotoColla" "$@"
