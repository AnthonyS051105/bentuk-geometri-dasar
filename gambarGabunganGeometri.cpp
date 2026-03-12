/*
 * ============================================================
 *  PEMANDANGAN SUNSET — Lima Bentuk Geometri Dasar
 * ============================================================
 *  Lima bentuk:
 *   1. LINGKARAN       → Matahari terbenam
 *   2. TRAPESIUM       → Gunung-gunung
 *   3. SEGITIGA        → Pohon-pohon pinus
 *   4. ELIPS           → Awan + kanopi pohon besar
 *   5. JAJARAN GENJANG → Jalur sawah / ladang
 *
 *  (+) PERSEGI PANJANG → Batang pohon pinus & pohon besar
 *
 *  Semua warna FLAT (tanpa gradasi).
 *
 *  Compile:  g++ -std=c++17 -O2 -o gambarGabunganGeometri gambarGabunganGeometri.cpp
 *  Run:      ./gambarGabunganGeometri
 *  View:     eog gambarGabunganGeometri.bmp
 * ============================================================
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

static const int W = 900;
static const int H = 650;

struct Color {
    uint8_t r, g, b;
    Color(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0) : r(r), g(g), b(b) {}
};

static Color canvas[H][W];

inline void putPixel(int x, int y, const Color& c)
{
    if (x >= 0 && x < W && y >= 0 && y < H) canvas[y][x] = c;
}

void fillRect(int x, int y, int w, int h, const Color& c)
{
    for (int j = std::max(0, y); j < std::min(H, y + h); j++)
        for (int i = std::max(0, x); i < std::min(W, x + w); i++)
            canvas[j][i] = c;
}

void fillCircle(int cx, int cy, int r, const Color& c)
{
    int r2 = r * r;
    for (int y = cy - r; y <= cy + r; y++)
        for (int x = cx - r; x <= cx + r; x++)
            if ((x - cx) * (x - cx) + (y - cy) * (y - cy) <= r2)
                putPixel(x, y, c);
}

void strokeCircle(int cx, int cy, int r, int thick, const Color& c)
{
    int ri = r - thick, ro = r;
    int ri2 = ri * ri, ro2 = ro * ro;
    for (int y = cy - ro - 1; y <= cy + ro + 1; y++)
        for (int x = cx - ro - 1; x <= cx + ro + 1; x++) {
            int d2 = (x - cx) * (x - cx) + (y - cy) * (y - cy);
            if (d2 >= ri2 && d2 <= ro2) putPixel(x, y, c);
        }
}

void fillEllipse(int cx, int cy, int rx, int ry, const Color& c)
{
    for (int y = cy - ry; y <= cy + ry; y++)
        for (int x = cx - rx; x <= cx + rx; x++) {
            float dx = (float)(x - cx) / rx;
            float dy = (float)(y - cy) / ry;
            if (dx * dx + dy * dy <= 1.0f)
                putPixel(x, y, c);
        }
}

using Pts = std::vector<std::pair<float, float>>;

void fillPolygon(const Pts& pts, const Color& c)
{
    int n = (int)pts.size();
    if (n < 3) return;
    float ymin = pts[0].second, ymax = pts[0].second;
    for (auto& p : pts) { ymin = std::min(ymin, p.second); ymax = std::max(ymax, p.second); }
    for (int y = (int)ymin; y <= (int)ymax; y++) {
        std::vector<float> xs;
        for (int i = 0; i < n; i++) {
            float x0 = pts[i].first, y0 = pts[i].second;
            float x1 = pts[(i+1)%n].first, y1 = pts[(i+1)%n].second;
            if (y0 == y1) continue;
            if (y >= std::min(y0,y1) && y < std::max(y0,y1))
                xs.push_back(x0 + (y - y0) / (y1 - y0) * (x1 - x0));
        }
        std::sort(xs.begin(), xs.end());
        for (int k = 0; k + 1 < (int)xs.size(); k += 2)
            for (int x = (int)xs[k]; x <= (int)xs[k+1]; x++)
                putPixel(x, y, c);
    }
}

void saveBMP(const char* fname)
{
    // BMP rows must be padded to multiples of 4 bytes
    int rowBytes = W * 3;
    int padding  = (4 - (rowBytes % 4)) % 4;
    int dataSize = (rowBytes + padding) * H;
    int fileSize = 54 + dataSize;

    std::ofstream f(fname, std::ios::binary);

    // ── BMP File Header (14 bytes) ──
    f.put('B'); f.put('M');
    auto writeLE32 = [&](int v) {
        f.put((char)(v      & 0xFF));
        f.put((char)((v>>8) & 0xFF));
        f.put((char)((v>>16)& 0xFF));
        f.put((char)((v>>24)& 0xFF));
    };
    auto writeLE16 = [&](int v) {
        f.put((char)(v     & 0xFF));
        f.put((char)((v>>8)& 0xFF));
    };
    writeLE32(fileSize);   // file size
    writeLE16(0);          // reserved1
    writeLE16(0);          // reserved2
    writeLE32(54);         // pixel data offset

    // ── DIB Header (BITMAPINFOHEADER, 40 bytes) ──
    writeLE32(40);         // header size
    writeLE32(W);          // width
    writeLE32(H);          // height (positive = bottom-up)
    writeLE16(1);          // color planes
    writeLE16(24);         // bits per pixel
    writeLE32(0);          // compression (none)
    writeLE32(dataSize);   // image data size
    writeLE32(2835);       // horizontal resolution (72 DPI)
    writeLE32(2835);       // vertical resolution
    writeLE32(0);          // colors in palette
    writeLE32(0);          // important colors

    // ── Pixel data (bottom-up, BGR order) ──
    char pad[3] = {0, 0, 0};
    for (int y = H - 1; y >= 0; y--) {
        for (int x = 0; x < W; x++) {
            f.put((char)canvas[y][x].b);
            f.put((char)canvas[y][x].g);
            f.put((char)canvas[y][x].r);
        }
        if (padding > 0) f.write(pad, padding);
    }
}

int main()
{
    // Palet warna (semua FLAT, tanpa gradasi)
    Color skyColor    (210, 155, 130);   // langit warm peach
    Color groundColor (140, 105,  60);   // tanah cokelat

    Color sunColor    (240, 210, 100);   // matahari kuning
    Color sunOutline  (200, 170,  60);   // garis tepi matahari

    Color mtnFar      ( 75,  90,  72);   // gunung jauh (hijau keabuan)
    Color mtnMid      ( 55,  72,  52);   // gunung tengah
    Color mtnNear     ( 40,  58,  38);   // gunung dekat

    Color pineColor   ( 22,  65,  38);   // pohon pinus hijau tua
    Color trunkColor  ( 50,  38,  22);   // batang pohon cokelat tua

    Color cloudColor  (225, 205, 165);   // awan krem hangat

    Color canopyColor ( 90,  68,  28);   // kanopi pohon besar

    Color furrowDark  ( 95,  70,  38);   // alur sawah gelap
    Color furrowLight (160, 125,  70);   // alur sawah terang

    Color pineBaseCol ( 30,  55,  32);   // strip hijau kaki pohon

    int horizonY = 310;

    // Latar belakang: 2 area flat 
    fillRect(0, 0, W, horizonY, skyColor);                // langit
    fillRect(0, horizonY, W, H - horizonY, groundColor);  // tanah

    //  BENTUK 1 — LINGKARAN — Matahari Terbenam
    //  fillCircle: jari-jari tunggal (r=80), dijamin bulat sempurna.
    fillCircle(450, 210, 80, sunColor);
    strokeCircle(450, 210, 80, 3, sunOutline);

    //  BENTUK 4 — ELIPS — Awan (masing-masing 1 elips saja)
    fillEllipse(680,  75, 85, 20, cloudColor);
    fillEllipse(195, 110, 60, 16, cloudColor);
    fillEllipse(540, 148, 45, 12, cloudColor);
    fillEllipse(100,  58, 50, 14, cloudColor);

    //  BENTUK 2 — TRAPESIUM — Gunung (masing-masing 1 trapesium, tanpa stroke)
    //  4 titik: puncak sempit di atas, alas lebar di bawah → trapesium.
    //  Tidak ada strokePolygon (penyebab garis putus-putus dihilangkan).
    
    // Gunung jauh kiri
    fillPolygon({{0.f,380.f},{160.f,218.f},{195.f,225.f},{310.f,380.f}}, mtnFar);
    // Gunung jauh tengah (besar)
    fillPolygon({{220.f,380.f},{420.f,188.f},{478.f,195.f},{680.f,380.f}}, mtnMid);
    // Gunung jauh kanan
    fillPolygon({{580.f,380.f},{745.f,208.f},{790.f,222.f},{900.f,380.f}}, mtnFar);
    // Gunung dekat kiri
    fillPolygon({{0.f,380.f},{90.f,278.f},{138.f,288.f},{265.f,380.f}}, mtnNear);
    // Gunung dekat kanan
    fillPolygon({{640.f,380.f},{762.f,268.f},{810.f,278.f},{900.f,380.f}}, mtnNear);

    //  BENTUK 3 — SEGITIGA — Pohon Pinus (+ PERSEGI PANJANG batang)
    //  Setiap pohon: 1 segitiga (tajuk) + 1 persegi panjang (batang).
    struct Pine { int cx, baseY, h, halfW; };
    Pine pines[] = {
        // baris belakang
        {155,380,55,18}, {225,378,60,20}, {295,380,52,17},
        {365,378,58,19}, {435,380,55,18}, {505,378,60,20},
        {575,380,52,17}, {645,378,58,19}, {715,380,55,18},
        // baris depan (lebih besar)
        {190,392,70,24}, {265,390,65,22}, {340,392,72,25},
        {415,390,68,23}, {490,392,72,25}, {565,390,65,22},
        {640,392,70,24}, {715,390,68,23},
    };
    for (auto& p : pines) {
        // Segitiga tajuk
        fillPolygon({
            {(float)p.cx,             (float)(p.baseY - p.h)},
            {(float)(p.cx - p.halfW), (float)p.baseY},
            {(float)(p.cx + p.halfW), (float)p.baseY}
        }, pineColor);
        // Persegi panjang batang
        fillRect(p.cx - 3, p.baseY, 6, 14, trunkColor);
    }

    // Strip hijau gelap menyatukan kaki pohon
    fillRect(0, 394, W, 10, pineBaseCol);

    //  BENTUK 5 — JAJARAN GENJANG — Jalur Sawah / Ladang
    //  4 titik: sisi atas & bawah sejajar, offset horizontal → jajaran genjang
    int fieldTop = 404;
    for (int i = 0; i < 8; i++) {
        int y0 = fieldTop + i * 30;
        int y1 = y0 + 16;
        float cx = 450.f;
        float halfW  = 80.f + i * 52.f;  // lebar atas = lebar bawah (sama)
        float shiftX = 18.f;             // geser horizontal → jajar genjang simetris
        Color fc = (i % 2 == 0) ? furrowDark : furrowLight;
        // Jajar genjang: sisi atas & bawah sejajar DAN sama panjang
        // Sisi kiri & kanan juga sejajar DAN sama panjang
        fillPolygon({
            {cx - halfW + shiftX, (float)y0},   // kiri atas
            {cx + halfW + shiftX, (float)y0},   // kanan atas
            {cx + halfW - shiftX, (float)y1},   // kanan bawah
            {cx - halfW - shiftX, (float)y1}    // kiri bawah
        }, fc);
    }

    // Pohon besar kiri (1 persegi panjang batang + 1 elips kanopi meninggi)
    fillRect(60, 220, 22, 430, trunkColor);
    fillEllipse(75, 145, 68, 140, canopyColor);

    // Pohon besar kanan (1 persegi panjang batang + 1 elips kanopi meninggi)
    fillRect(808, 195, 24, 455, trunkColor);
    fillEllipse(820, 120, 72, 145, canopyColor);

    // Simpan
    saveBMP("gambarGabunganGeometri.bmp");

    std::cout << "Gambar berhasil disimpan: gambarGabunganGeometri.bmp (" << W << "x" << H << ")\n";

    return 0;
}
