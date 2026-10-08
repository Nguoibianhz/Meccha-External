# 🦎 Meccha-External (V3.6)

> **MecchaChameleon External Game Overlay & Visual Assistant**  
> **Tác giả / Author:** **Nguyễn Mạnh Hiếu**  
> **Phiên bản:** `V3.6` (Build x64 Release)

---

## 📖 Giới thiệu (Overview)
**Meccha-External** là dự án external overlay hiệu năng cao chạy ở User-mode, được thiết kế chuyên biệt cho việc render giao diện đồ họa Direct3D 11 composited mượt mà (hỗ trợ hiển thị lên tới 240 FPS) kết hợp với thư viện Dear ImGui được tùy biến cao cấp (Modern Slate Theme).

Ứng dụng hỗ trợ cơ chế **Smart Cursor Passthrough**, ký số Authenticode độc quyền giúp hạn chế false positive từ Windows Defender/Antivirus, và tối ưu hóa đọc bộ nhớ không xâm lấn (non-invasive memory reading).

---

## 🛠️ Cấu trúc mã nguồn (Project Architecture)

Cấu trúc dự án được phân cấp rõ ràng theo mô hình Module / Engine / Manager:

```text
Meccha-External/
├── Assets/                                    # Tài nguyên hình ảnh, logo, demo
├── MecchaChameleon/                           # Thư mục giải pháp (Solution folder)
│   ├── MecchaChameleon.slnx                   # File solution Visual Studio
│   └── MecchaChameleon/                       # Thư mục Project C++
│       ├── MecchaChameleon.vcxproj            # Project file (MSVC v143)
│       ├── MecchaChameleon.rc                 # Windows Resource (Metadata, Version 3.6.0.0)
│       ├── app.manifest                       # Manifest (Per-Monitor V2 DPI Aware, Invoker)
│       ├── app.ico                            # Icon ứng dụng (Multi-resolution)
│       └── MecchaChameleon/                   # Mã nguồn gốc C++
│           ├── main.cpp                       # Điểm khởi chạy, vòng lặp chính & cursor passthrough
│           ├── Engine/                        # Thư viện đồ họa và giao tiếp hệ thống
│           │   ├── ImGui/                     # Dear ImGui core & custom controls
│           │   ├── Memory/                    # RPM/WPM Process Memory wrapper
│           │   ├── MecchaChameleon/           # Engine logic, game state & actor tracking
│           │   └── LogoData.hpp               # Dữ liệu Texture Logo nén nhúng trực tiếp
│           ├── Manager/                       # Quản lý vòng đời và trạng thái toàn cục
│           │   ├── Classmanager/              # Dependency injection & lifecycle manager
│           │   └── Globals/                   # Cấu hình cài đặt toàn cục (Globals.hpp)
│           └── Modules/                       # Các tính năng chức năng độc lập
│               ├── Overlay/                   # Direct3D 11 Transparent Overlay window
│               ├── Menu/                      # Giao diện điều khiển ImGui hiện đại
│               ├── ESP/                       # 2D Box, Skeleton, Snaplines, Minimap Radar
│               └── Aimbot/                    # Smooth Aiming, FOV calculation, target selection
├── logo.png                                   # Logo thiết kế chính thức của dự án
├── sign_executable.ps1                        # Script tự động ký số Authenticode SHA-256
├── LICENSE                                    # Giấy phép bản quyền nguồn đóng / phi thương mại
└── README.md                                  # Hướng dẫn chi tiết dự án
```

---

## ✨ Tính năng nổi bật (Features)

1. **Giao diện Modern Slate Theme (ImGui v3.6):**
   - Thiết kế dạng thẻ card hiện đại, bố cục trực quan: Combat, Visuals, Settings.
   - Nhúng logo trực tiếp trên thanh Header bằng Direct3D 11 Shader Resource.
   - Hỗ trợ đổi màu tức thì thông qua Color Picker / Palette Button.

2. **Smart Cursor Passthrough:**
   - Khi Menu đang mở: Nếu chuột rê ra ngoài cửa sổ Menu, hệ thống tự động cho phép click xuyên qua để thao tác với các tab trình duyệt hoặc ứng dụng khác mà không cần ẩn Menu.

3. **ESP & Visuals Hoàn thiện:**
   - **2D Box ESP:** Tự động fallback kích thước, tính toán bounding box chính xác.
   - **Skeleton ESP:** Vẽ khung xương nhân vật.
   - **Snaplines (Tracers):** Hỗ trợ 3 chế độ xuất phát: `Bot` (chân), `Center` (tâm), `Top` (đỉnh).
   - **Minimap Radar, FoV Circle & Chinese Hat.**

4. **Phím tắt điều khiển:**
   - Phím mở/tắt Menu: **`RIGHT SHIFT`** (Shift phải).

---

## 🚀 Hướng dẫn sử dụng (How to Use)

### 1. Sử dụng trực tiếp từ bản Release (Pre-built)
1. Tải file `MecchaChameleon.exe` từ tab **[Releases](https://github.com/Nguoibianhz/Meccha-External/releases)**.
2. Khởi chạy game mục tiêu.
3. Chạy file `MecchaChameleon.exe` với quyền Administrator (nếu cần thiết).
4. Nhấn phím **`RIGHT SHIFT`** trên bàn phím để bật / tắt giao diện điều khiển.

### 2. Tự biên dịch từ mã nguồn (Build from Source)
* **Yêu cầu môi trường:**
  * Visual Studio 2022 (v143 Toolset) hoặc Visual Studio Build Tools.
  * Windows SDK `10.0.22621.0` hoặc `10.0.26100.0`.
  * C++20 Standard.

* **Biên dịch qua dòng lệnh:**
```powershell
# Chạy MSBuild ở cấu hình Release x64
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" `
    "MecchaChameleon\MecchaChameleon\MecchaChameleon.vcxproj" `
    /p:Configuration=Release /p:Platform=x64 /v:m
```

* **Ký số Authenticode chống False Positive:**
```powershell
powershell -ExecutionPolicy Bypass -File .\sign_executable.ps1
```

---

## 📜 Điều khoản bản quyền & Giấy phép (License & Attribution)

**Bản quyền (c) 2026 Nguyễn Mạnh Hiếu. Toàn quyền bảo lưu.**

Dự án này được phát hành đi kèm giấy phép bản quyền nghiêm ngặt tại tệp [`LICENSE`](./LICENSE):

- ❌ **NGHIÊM CẤM THƯƠNG MẠI HÓA:** Nghiêm cấm tuyệt đối hành vi mua bán, kinh doanh, thương mại hóa mã nguồn hoặc file thực thi (binary) khi chưa có sự đồng ý bằng văn bản của tác giả **Nguyễn Mạnh Hiếu**.
- ⚠️ **BẮT BUỘC GHI CÔNG (ATTRIBUTION REQUIRED):** Mọi hành vi remake, phát triển tiếp, sử dụng lại mã nguồn đều **BẮT BUỘC** phải ghi rõ tên tác giả: **Nguyễn Mạnh Hiếu** và gắn kèm link đến repository gốc này.
- 🎓 **MỤC ĐÍCH SỬ DỤNG:** Dự án chỉ phục vụ mục đích học tập, nghiên cứu kiến trúc Windows Internals và Direct3D Game Overlay.
