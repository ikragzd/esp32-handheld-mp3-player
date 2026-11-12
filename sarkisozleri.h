#ifndef SARKISOZLERI_H
#define SARKISOZLERI_H

#include <Arduino.h>

struct SarkiSozu {
    unsigned long baslamaZamani; // Milisaniye cinsinden
    unsigned long bitisZamani;   // Milisaniye cinsinden
    const char* metin;
};

// ==========================================
// ŞARKI SÖZLERİ ZAMAN ÇİZELGESİ
// ==========================================

// Örnek: Şarkı 1 için (I just wanna be yours)
const SarkiSozu sarki1_sozleri[] = {
    {15000, 22000, "I wanna be your vacuum cleaner"}, // 15. saniye ile 20. saniye arası
    {22000, 28000, "Breathing in your dust"},         // 22. ile 28. saniye arası
    {0, 0, nullptr} // DİZİNİN BİTTİĞİNİ ANLAMASI İÇİN BU SATIR HEP EN SONDA OLMALI
};

// Örnek: Şarkı 2 için
const SarkiSozu sarki2_sozleri[] = {
    {197000, 201000, "Seninle"},
    {201000, 204000, "Sevişmek"},
    {204000, 213000,"İstiyorum..."},
    {0, 0, nullptr}
};
const SarkiSozu sarki3_sozleri[] = {
    {53000, 59000, "Ve çok çok önce yenilmişim sana"},
    {59000, 66000, "Bilmeyerek..."},
    {0, 0, nullptr}
};
const SarkiSozu sarki5_sozleri[] = {
    {65000, 67000, "Won't you kiss me on the mouth"},
    {67000,70000, "And love me like a sailor?"},
    {70000, 72000, "And when you get a taste"},
    {72000, 75000, "Can you tell me what's my flavor?"},
     {202000, 209000, "We can go forever until you wanna sit it out..."},
    {0, 0, nullptr}
};
const SarkiSozu sarki6_sozleri[] = {
    {22000, 27000, "Belki bir şarkının her sesinde..."},
    {48000,55000, "Bambaşka bir şeyi düşünürken aklımdasın"},
    {116000, 120000, "Koparmadan çiçek koklar gibi..."},
    {0, 0, nullptr}
};
const SarkiSozu sarki10_sozleri[] = {
    {72000, 77000, "Ah sevgilim alıp götürsen bizi"},
    {77000, 83000, "Silinir mi hafızamdan ellerinin izi"},
    {0, 0, nullptr}
};
const SarkiSozu sarki11_sozleri[] = {
    {39000, 43000, "Seni sorana her yanım derim"},
    {43000, 46000, "Ve dahasını da eklerim"},
     {46000, 50000, "Ellerini uzat ki dokunsun"},
      {50000, 52000, "Parmaklarım..."},
    {0, 0, nullptr}
};
const SarkiSozu sarki12_sozleri[] = {
    {20000, 26000, "Dolaşırdı adımlarım birbirine seni görünce"},
    {139000, 141000, "Eriyorum karşında..."},
    {141000, 145000, "Yerinde olsam nasıl öperdim"},
    {0, 0, nullptr}
};

// ==========================================
// ANA ARAMA FONKSİYONU
// ==========================================
const SarkiSozu* getSenkronizeSozler(int kategori, int sarkiNo) {
    if (kategori == 0) { // ROMANTIK Klasörü (01)
        switch(sarkiNo) {
            case 1: return sarki1_sozleri;
            case 2: return sarki2_sozleri;
            case 3: return sarki3_sozleri;
            case 5: return sarki5_sozleri;
            case 6: return sarki6_sozleri;
            case 10: return sarki10_sozleri;
            case 11: return sarki11_sozleri;
            case 12: return sarki12_sozleri;
            default: return nullptr;
        }
    } 
    else if (kategori == 1) { // KARISIK Klasörü (02)
        switch(sarkiNo) {
            // Örnek: case 1: return karisik_sarki1_sozleri;
            default: return nullptr;
        }
    }
    else if (kategori == 2) { // EGLENCELI Klasörü (03)
        switch(sarkiNo) {
            default: return nullptr;
        }
    }
    else if (kategori == 3) { // KLASIK Klasörü (04)
        switch(sarkiNo) {
            default: return nullptr;
        }
    }

    return nullptr; // Söz bulamazsa boş döner
}

#endif
