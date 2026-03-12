#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

static const int W = 900;
static const int H = 650;

struct Color { uint8_t r, g, b; };
using Pts = std::vector<std::pair<float, float>>;

static Color canvas[H][W];

void putPixel(int x, int y, const Color& c) {
    if (x >= 0 && x < W && y >= 0 && y < H) canvas[y][x] = c;
}

void fillRect(int x, int y, int w, int h, const Color& c) {
    for (int j = std::max(0, y); j < std::min(H, y + h); j++)
        for (int i = std::max(0, x); i < std::min(W, x + w); i++)
            canvas[j][i] = c;
}

void fillCircle(int cx, int cy, int r, const Color& c) {
    int r2 = r * r;
    for (int y = cy - r; y <= cy + r; y++)
        for (int x = cx - r; x <= cx + r; x++)
            if ((x - cx) * (x - cx) + (y - cy) * (y - cy) <= r2)
                putPixel(x, y, c);
}

void strokeCircle(int cx, int cy, int r, int thick, const Color& c) {
    int ri2 = (r - thick) * (r - thick), ro2 = r * r;
    for (int y = cy - r - 1; y <= cy + r + 1; y++)
        for (int x = cx - r - 1; x <= cx + r + 1; x++) {
            int d2 = (x - cx) * (x - cx) + (y - cy) * (y - cy);
            if (d2 >= ri2 && d2 <= ro2) putPixel(x, y, c);
        }
}

void fillEllipse(int cx, int cy, int rx, int ry, const Color& c) {
    for (int y = cy - ry; y <= cy + ry; y++)
        for (int x = cx - rx; x <= cx + rx; x++) {
            float dx = (float)(x - cx) / rx, dy = (float)(y - cy) / ry;
            if (dx * dx + dy * dy <= 1.0f) putPixel(x, y, c);
        }
}

void fillPolygon(const Pts& pts, const Color& c) {
    int n = (int)pts.size();
    if (n < 3) return;
    float ymin = pts[0].second, ymax = pts[0].second;
    for (auto& p : pts) { ymin = std::min(ymin, p.second); ymax = std::max(ymax, p.second); }
    for (int y = (int)ymin; y <= (int)ymax; y++) {
        std::vector<float> xs;
        for (int i = 0; i < n; i++) {
            float x0 = pts[i].first, y0 = pts[i].second;
            float x1 = pts[(i + 1) % n].first, y1 = pts[(i + 1) % n].second;
            if (y0 == y1) continue;
            if (y >= std::min(y0, y1) && y < std::max(y0, y1))
                xs.push_back(x0 + (y - y0) / (y1 - y0) * (x1 - x0));
        }
        std::sort(xs.begin(), xs.end());
        for (int k = 0; k + 1 < (int)xs.size(); k += 2)
            for (int x = (int)xs[k]; x <= (int)xs[k + 1]; x++)
                putPixel(x, y, c);
    }
}

void drawTree(int trunkX, int trunkY, int trunkW, int trunkH,
              int canopyCx, int canopyCy, int rx, int ry,
              const Color& trunk, const Color& canopy) {
    fillRect(trunkX, trunkY, trunkW, trunkH, trunk);
    fillEllipse(canopyCx, canopyCy, rx, ry, canopy);
}

void saveBMP(const char* fname) {
    int rowBytes = W * 3;
    int padding  = (4 - (rowBytes % 4)) % 4;
    int dataSize = (rowBytes + padding) * H;

    std::ofstream f(fname, std::ios::binary);
    auto w32 = [&](int v) {
        f.put(v & 0xFF); f.put((v >> 8) & 0xFF);
        f.put((v >> 16) & 0xFF); f.put((v >> 24) & 0xFF);
    };
    auto w16 = [&](int v) { f.put(v & 0xFF); f.put((v >> 8) & 0xFF); };

    f.put('B'); f.put('M');
    w32(54 + dataSize); w16(0); w16(0); w32(54);
    w32(40); w32(W); w32(H); w16(1); w16(24);
    w32(0); w32(dataSize); w32(2835); w32(2835); w32(0); w32(0);

    char pad[3] = {};
    for (int y = H - 1; y >= 0; y--) {
        for (int x = 0; x < W; x++) {
            f.put(canvas[y][x].b); f.put(canvas[y][x].g); f.put(canvas[y][x].r);
        }
        if (padding > 0) f.write(pad, padding);
    }
}

int main() {
    Color sky{210,155,130}, ground{140,105,60};
    Color sun{240,210,100}, sunEdge{200,170,60};
    Color mtnFar{75,90,72}, mtnMid{55,72,52}, mtnNear{40,58,38};
    Color pine{22,65,38}, trunk{50,38,22};
    Color cloud{225,205,165}, canopy{90,68,28};
    Color furrowA{95,70,38}, furrowB{160,125,70};
    Color pineBase{30,55,32};

    fillRect(0, 0, W, 310, sky);
    fillRect(0, 310, W, H - 310, ground);

    // Matahari (lingkaran)
    fillCircle(450, 210, 80, sun);
    strokeCircle(450, 210, 80, 3, sunEdge);

    // Awan (elips)
    struct Awan { int cx, cy, rx, ry; };
    Awan awans[] = {{680,75,85,20},{195,110,60,16},{540,148,45,12},{100,58,50,14}};
    for (auto& a : awans) fillEllipse(a.cx, a.cy, a.rx, a.ry, cloud);

    // Gunung (trapesium)
    struct Mtn { Pts pts; Color c; };
    std::vector<Mtn> mtns = {
        {{{0,380},{160,218},{195,225},{310,380}}, mtnFar},
        {{{220,380},{420,188},{478,195},{680,380}}, mtnMid},
        {{{580,380},{745,208},{790,222},{900,380}}, mtnFar},
        {{{0,380},{90,278},{138,288},{265,380}}, mtnNear},
        {{{640,380},{762,268},{810,278},{900,380}}, mtnNear}
    };
    for (auto& m : mtns) fillPolygon(m.pts, m.c);

    // Pohon pinus (segitiga + persegi panjang)
    struct Pine { int cx, baseY, h, halfW; };
    Pine pines[] = {
        {155,380,55,18},{225,378,60,20},{295,380,52,17},{365,378,58,19},
        {435,380,55,18},{505,378,60,20},{575,380,52,17},{645,378,58,19},{715,380,55,18},
        {190,392,70,24},{265,390,65,22},{340,392,72,25},{415,390,68,23},
        {490,392,72,25},{565,390,65,22},{640,392,70,24},{715,390,68,23}
    };
    for (auto& p : pines) {
        fillPolygon({{(float)p.cx,(float)(p.baseY-p.h)},{(float)(p.cx-p.halfW),(float)p.baseY},{(float)(p.cx+p.halfW),(float)p.baseY}}, pine);
        fillRect(p.cx - 3, p.baseY, 6, 14, trunk);
    }

    fillRect(0, 394, W, 10, pineBase);

    // Jalur sawah (jajaran genjang)
    for (int i = 0; i < 8; i++) {
        int y0 = 404 + i * 30, y1 = y0 + 16;
        float cx = 450.f, hw = 80.f + i * 52.f, sx = 18.f;
        fillPolygon({
            {cx-hw+sx,(float)y0},{cx+hw+sx,(float)y0},
            {cx+hw-sx,(float)y1},{cx-hw-sx,(float)y1}
        }, i % 2 == 0 ? furrowA : furrowB);
    }

    // Pohon besar (persegi panjang batang + elips kanopi)
    drawTree(60, 220, 22, 430, 75, 145, 68, 140, trunk, canopy);
    drawTree(808, 195, 24, 455, 820, 120, 72, 145, trunk, canopy);

    saveBMP("gambarGabunganGeometri.bmp");
    return 0;
}
