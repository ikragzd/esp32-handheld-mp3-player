#include "animasyonlar.h" // Hex kodlarımızın olduğu çekmece
#include "sarkisozleri.h"
#include <Adafruit_GFX.h>
#include <Fonts/TomThumb.h>
#include <Adafruit_SSD1306.h>
#include <DFRobotDFPlayerMini.h>
#include <SPI.h>
#include <Wire.h>
#include <esp_sleep.h>
#include <vector>

// ==========================================
// 1. EKRAN VE PİN AYARLARI
// ==========================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SDA_PIN 5 // Yeni SDA pinin
#define SCL_PIN 4 // Yeni SCL pinin
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

HardwareSerial mySerial(1); // UART1 (önceden çalışan ayar)
DFRobotDFPlayerMini myDFPlayer;

// Buton Pinleri
const int btnIleri = 19; // İleri / Ses+
const int btnGeri = 6;   // Geri / Ses-
const int btnOynat = 7;  // Oynat-Durdur / Kapanış (Uzun Basım)

// ==========================================
// 2. SİSTEM DEĞİŞKENLERİ
// ==========================================
enum Durum {
  GIRIS,
  DURUM_MENU,
  DURUM_OYNATMA,
  DURUM_AYAR_UYKU,
  DURUM_AYAR_GECE,
  DURUM_AYAR_SES,
  KAPANIS
};
Durum sistemDurumu = GIRIS; // Giriş animasyonu ile başla

const char *menuSecenekleri[] = {
    "< ROMANTIK >",  "< KARISIK >",   "< EGLENCELI >", "< KLASIK >",
    "< UYKU MODU >", "< GECE MODU >", "< SES >"};
int seciliMenu = 0;
int aktifKategori = 0; // 0: Romantik, 1: Karışık, 2: Eğlenceli, 3: Klasik
int sonSarkilar[4] = {1, 1, 1,
                      1}; // Her kategori için son kalınan şarkı numarası

const char *uykuSecenekleri[] = {"< KAPALI >", "< 15 DK >", "< 30 DK >",
                                 "< 45 DK >", "< 60 DK >"};
int uykuSureDakika[] = {0, 15, 30, 45, 60};
int uykuSecim = 0;
unsigned long uykuBaslamaZamani = 0;
unsigned long uykuHedefSuresi = 0;

const char *geceSecenekleri[] = {"< KAPALI >", "< ACIK >"};
int geceSecim = 0;

const char *sesSecenekleri[] = {"< 10 >", "< 15 >", "< 20 >", "< 25 >",
                                "< 30 >"};
int sesDegerleri[] = {10, 15, 20, 25, 30};
int sesSecim = 1; // Başlangıçta 15 (1. indeks), hoparlör cızırtısını ve
                  // ısınmayı önlemek için

int suAnkiSahne = 1;

// KLON ÇİP SAYIM YAPAMADIĞI İÇİN BURAYA KLASÖRLERDEKİ GERÇEK ŞARKI SAYILARINIZI
// GİRİN: Sırasıyla: Romantik, Karışık, Eğlenceli, Klasik
int gercekSarkiSayilari[4] = {
    23, 29, 18,
    13}; // İleride klasörlere yeni MP3 eklerseniz buradaki sayıları artırın!

int klasorSarkiSayilari[4] = {99, 99, 99, 99}; // Kurulumda güncellenecek

bool sarkiCaliyorMu = false;

// Zamanlama ve Senkronizasyon Değişkenleri
unsigned long sarkiBaslamaMilis = 0;
unsigned long sarkiDurdurmaMilis = 0;

// Ses Kontrol Değişkenleri
int suAnkiSes = 15; // Başlangıç ses seviyesi
int MAX_SES = 15; // Maksimum ses sınırı (Hoparlör için varsayılan 15, kulaklık
                  // için menüden artırılabilir)
unsigned long sesGosterimZamani = 0; // Ekranda sesin gösterilme süresi

// ==========================================
// ALT BİLGİ ÇUBUĞU YAZILARI
// Her şarkı için ekranda görünecek yazıyı buradan ayarlayabilirsin.
// Şarkı adını, sözünü veya güzel bir söz yazabilirsin.
// ==========================================
std::vector<String> sarkiYazilari[4] = {
    // 0: ROMANTIK (Klasör 01)
    {
        "I Wanna Be Yours -   Artic Monkeys",     // 1
        "Bebek-    Alkan Dalgakıran",             // 2
        "Sayende-  Pilli Bebek",                  // 3
        "Kal(Feat. Teoman)- Atiye",               // 4
        "Sailor Song-Gigi Perez",                 // 5
        "Sen Benim Şarkılarımsın -  Gündoğarken", // 6
        "Al Beni-     Kalben<3",                  // 7
        "Ay Tenli Kadın-Ufuk Beydemir",           // 8
        "Ojos Tristes-  Selena Gomez",            // 9
        "Ellerinin İzi-Selin Çıngır",             // 10
        "Edepsiz Komedya-  Sakin",                // 11
        "Bihaber - Vega",                         // 12
        "WILDFLOWER-Billie Eilish",               // 13
        "Wicked Game-Chris Isaak",                // 14
        "Bal-Duman",                              // 15
        "Teessüf Ederim -  Duman",                   // 16
        "Put Your Head On My Sholders",           // 17
        "Ne Güzel Güldün -  Pinhani",                // 18
        "Bikinisin-de Astronomi-SFC",              // 19
        "Shape Of My Heart -   Sting",                // 20
        "House of Rising Sun",                    // 21
        "Golden Brown-The Stranglers",            // 22
        "Aşk Durdukça- Yüksek Sadakat"             // 23
    },
    // 1: KARISIK (Klasör 02)
    {
        "Remember The Rain-21st Century",   // 1
        "Birds Of A Feather-  Billie Eilish", // 2
        "Ilomilo-  Billie Eilish",          // 3
        "Eylül Akşamı -  Pilli Bebek",      // 4
        "Dünya-Redd",                       // 5
        "Ara Sıra Bazı Bazı- Nilüfer",      // 6
        "Hep de Yorgun -  Yaşlı Amca",      // 7
        "Serzenişte-Vega",                  // 8
        "Prensesin Uykusuyum-Redd",         // 9
        "Fotoğraf- Pilli Bebek",             // 10
        "Yara -    Kalben",                      // 11
        "Olsun -   Pilli Bebek",                // 12
        "Hala Aşk Var Mı?-Redd",            // 13
        "Düşünürüm-Kalben",                 // 14
       "Bak - Pilli Bebek",
"Rüzgar - Kalben" ,
"Haykıracak Nefesim Kalmasa Bile <3",
"Kim Ne Derse Desin -Ajda Pekkan",
"Ölüm Kapımı Çalmasa Da - Aslı Gökyokuş",
"Anlamazdın - Ayla Dikmen",
"Wicked Game - Chris Isaak ",
"Falan Filan - Redd",
"Sevmeden Geçer Zaman - Redd",
"Seninle Bir Dakika - Semiha Yankı",
"Neredeysen - Semiramis",
"Bu Aşk Fazla Sana - Şebnem Ferah",
"Beni Anlama - Tarkan ",
"Every Breath You Take - The Police",
"İstanbul Beyefendisi - Yaşlı Amca"

    },
    // 2: EGLENCELI (Klasör 03)
    {
        "Kandırdım-Kenan Doğulu",            // 1
        "Ölürüm Sana -    Tarkan",                // 2
        "Adını Çıkar Deliye-   Barış Akarsu",   // 3
        "Duvet-Boa",                         // 4
        "Fırtınalar-Ebru Gündeş",            // 5
        "Can't Take My Eyes Of You...",      // 6
        "Çakkıdı-  Kenan Doğulu",              // 7
        "Doktor -  Kenan Doğulu",               // 8
        "Aya Benzer-   Mustafa Sandal",         // 9
        "Isyankar- Mustafa Sandal",           // 10
        "Insaf -   Ozgun",                       // 11
        "Mecbursun-Sertap Erener",           // 12
        "Hepsi Senin Mi?-Tarkan",            // 13
        "Simarik-  Tarkan",                    // 14
        "California Dreamin'",               // 15
        "Paint It Black-The Rolling Stones", // 16
        "Be My Baby-The Ronetters",          // 17
        "Lovers Rock-TV Girl"                // 18
    },
    // 3: KLASIK (Klasör 04)
    {
        // 2
    }};

// Zaman ve Animasyon Motoru
unsigned long sonKareZamani = 0;
int suAnkiKare = 0;

// ==========================================
// ANİMASYON HIZ AYARLARI (ms cinsinden)
// İstediğin animasyonun hızını buradan değiştir
// ==========================================
const int HIZI_GIRIS = 200; // Giriş animasyonu kare hızı (ms)
const int HIZI_GIRIS_SONKARE =
    1500;                     // Giriş SON KARE süresi (daha uzun durması için)
const int HIZI_KAPANIS = 400; // Kapanış animasyonu kare hızı (yavaşlatıldı)
const int HIZI_KAPANIS_SONKARE =
    3000;                       // Kapanış SON KARE süresi (3 saniye bekler)
const int HIZI_KEDI = 200;      // Kedi animasyonu kare hızı
const int HIZI_MUZIK = 250;     // Müzik animasyonu kare hızı
const int HIZI_KENARKEDI = 200; // Kenar kedi kare hızı
const int HIZI_BALIK = 180;     // Balık animasyonu kare hızı
const int HIZI_PLAK = 150;      // Plak animasyonu kare hızı
const int HIZI_KEDUSLAR = 250;  // Keduşlar animasyonu kare hızı
const int HIZI_OPUSEN = 200;    // Öpüşen çift kare hızı
const int HIZI_SARILAN = 200;   // Sarılan çift kare hızı
const int HIZI_PIANO = 300;
const int HIZI_KEMAN = 300;     // Keman animasyonu kare hızı
// ==========================================
// ANİMASYON DÖNGÜSÜ AYARLARI
// Animasyonlar bu sırayla ve bu sürelerde döner
// Tipler: 0=Kedi, 1=Muzik, 2=KenarKedi, 3=Balik, 4=Ask, 5=Plak, 6=Keduşlar,
// 7=OpusenCift, 8=SarilanCift, 9=Piano, 10=Keman
// ==========================================
const int ANIMASYON_SIRASI[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
const unsigned long ANIMASYON_SURESI[] = {
    8000, 8000, 8000, 8000,  5000,
    8000, 8000, 5000, 15000, 5000, 8000}; 
const int TOPLAM_ANIMASYON = 11;

int suAnkiAnimasyon = 0;                  // Şu anki animasyon indeksi
unsigned long animasyonBaslamaZamani = 0; // Bu animasyonun başlama zamanı

// Buton Zamanlayıcıları
unsigned long btnIleriZaman = 0, btnGeriZaman = 0, btnOynatZaman = 0;
bool btnIleriBasili = false, btnGeriBasili = false, btnOynatBasili = false;
bool btnIleriUzun = false, btnGeriUzun = false, btnOynatUzun = false;

// Aşk resminin döndürülmüş hali (128x64 = 16 byte/satır x 64 satır)
uint8_t askRotated[1024];

void sarkiBaslat(int sahne) {
  // Kullanıcının belirttiği gibi standart klasör formatı (örn: Klasör 01, Şarkı
  // 001.mp3) kullanılıyor. aktifKategori 0 ise (Romantik), +1 ile 1. klasör
  // olur. sahne 1 ise 1. şarkı olur. -> (1, 1) gönderilir.
  myDFPlayer.playFolder(aktifKategori + 1, sahne);
  sarkiCaliyorMu = true;
  sarkiBaslamaMilis = millis();

  // Şarkı başlarken animasyonu seç
  if (aktifKategori == 3) {
    suAnkiAnimasyon = random(9, 11); // Klasik için 9 (Piyano) veya 10 (Keman)
  } else {
    // Diğer kategoriler için 9 ve 10 hariç tam ekran olmayan rastgele bir animasyon seç
    suAnkiAnimasyon = random(0, TOPLAM_ANIMASYON - 2); // 0-8 arası seçer (9 ve 10 hariç)
    int animTipi = ANIMASYON_SIRASI[suAnkiAnimasyon];
    while (animTipi == 4 || animTipi == 6 || animTipi == 8 || animTipi >= 9) {
      suAnkiAnimasyon = random(0, TOPLAM_ANIMASYON - 2);
      animTipi = ANIMASYON_SIRASI[suAnkiAnimasyon];
    }
  }
  animasyonBaslamaZamani = millis();
  suAnkiKare = 0;
}

// ==========================================
// 3. SETUP (KURULUM)
// ==========================================
void setup() {
  delay(1000);
  Serial.begin(115200);

  // Pin Modları
  pinMode(btnIleri, INPUT_PULLUP);
  pinMode(btnGeri, INPUT_PULLUP);
  pinMode(btnOynat, INPUT_PULLUP);

  // I2C Pinlerini Manuel Yönlendir ve Ekranı Başlat
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000); // Stabil hız (400KHz çökmesine neden oluyordu!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setRotation(0); // EKRAN TERS ÇEVRİLDİ (Baş aşağı lehimleme için)
  display.clearDisplay();
  display.display(); // Ekranı temizle (karıncalanmayı önle!)

  // ESP32 RX/TX Pinlerini Tanıt
  mySerial.begin(9600, SERIAL_8N1, 1, 0);

  // === DFPLAYER BAŞLATMA ===
  delay(2000); // DFPlayer'a tam hazırlanma süresi

  // ACK KAPALI (donmayı önler)
  myDFPlayer.begin(mySerial, false, false);
  Serial.println("DFPlayer baslatildi");

  delay(500); // Klon çiplerin başlatma süresi uzun sürebilir, bekliyoruz

  myDFPlayer.volume(suAnkiSes);
  Serial.println("Ses ayarlandi");

  // === DİNAMİK KLASÖR ŞARKI SAYISI OKUMA ===
  // DFPlayer donanımsal olarak klasör okuma özürlüsü olduğu için (kart
  // formatlansa bile 2 diyor), mecburen sizin koda elinizle girdiğiniz
  // 'gercekSarkiSayilari' dizisini kullanıyoruz.
  delay(500);
  for (int i = 0; i < 4; i++) {
    klasorSarkiSayilari[i] = gercekSarkiSayilari[i];
    if (klasorSarkiSayilari[i] <= 0)
      klasorSarkiSayilari[i] = 1; // Sıfır koruması
  }

  delay(200);
  // sarkiBaslat(1); // BAŞLANGIÇTA ÇALMAYACAK, MENÜDEN SEÇİLİNCE BAŞLAYACAK
  // Serial.println("Sarki 1 baslatildi");

  // Aşk resmini 90 derece döndürüp hafızaya kaydet (tek seferlik, ağır iş
  // burada yapılır)
  memset(askRotated, 0, sizeof(askRotated));
  for (int row = 0; row < 128; row++) {
    for (int col = 0; col < 64; col++) {
      if (ask_anim[0][row * 8 + (col / 8)] & (128 >> (col & 7))) {
        int newX = row;      // Saat yönünde 90 derece
        int newY = 63 - col; // Ters çevrildi!
        askRotated[newY * 16 + (newX / 8)] |= (128 >> (newX & 7));
      }
    }
  }

  // Giriş animasyonu ile başla
  sistemDurumu = GIRIS;
  suAnkiKare = 0;
  sonKareZamani = millis();
}

// ==========================================
// SES ANİMASYONLARI KONTROLÜ
// ==========================================
bool gosterSesAcmaAnimasyonu = false;
bool gosterSesKismaAnimasyonu = false;

int getPilYuzdesi() {
  static float smoothedYuzde = -1; // Titremeyi önlemek için hafıza

  // Pin 3 üzerinden gerilim bölücü ile pil okuması
  int rawValue = analogRead(3); 
  
  // ÖNEMLİ: Bu değerleri kendi kullandığınız direnç (voltage divider) değerlerine
  // göre ince ayar (kalibre) etmeniz gerekebilir. 
  int anlikYuzde = map(rawValue, 2000, 3000, 0, 100); 
  
  if (anlikYuzde > 100) anlikYuzde = 100;
  if (anlikYuzde < 0) anlikYuzde = 0;

  if (smoothedYuzde == -1) {
    smoothedYuzde = anlikYuzde; // İlk başta direkt al
  } else {
    // Low-Pass Filter: Rakamların 64-65 arası git gel yapmasını engeller
    smoothedYuzde = (smoothedYuzde * 0.95) + (anlikYuzde * 0.05);
  }

  return (int)smoothedYuzde;
}

int getMenuKaresi(unsigned long suAn) {
  static unsigned long sonMenuKareZamani = 0;
  static int menuKaresi = 0;
  static bool menuIlkDonguBitti = false;
  static int sonSistemDurumu = -1;

  if (sonSistemDurumu != sistemDurumu) {
    if ((sonSistemDurumu != DURUM_MENU && sonSistemDurumu != DURUM_AYAR_UYKU &&
         sonSistemDurumu != DURUM_AYAR_GECE &&
         sonSistemDurumu != DURUM_AYAR_SES) &&
        (sistemDurumu == DURUM_MENU || sistemDurumu == DURUM_AYAR_UYKU ||
         sistemDurumu == DURUM_AYAR_GECE || sistemDurumu == DURUM_AYAR_SES)) {
      // Menüye yeni girildi! Baştan başlat
      menuKaresi = 0;
      menuIlkDonguBitti = false;
      sonMenuKareZamani = suAn;
    }
    sonSistemDurumu = sistemDurumu;
  }

  if (suAn - sonMenuKareZamani >= 300) { // Menü animasyonu hızı düşürüldü
    if (!menuIlkDonguBitti) {
      menuKaresi++;
      if (menuKaresi > 7) {
        menuIlkDonguBitti = true;
        menuKaresi = 6;
      }
    } else {
      // İlk tur bitti, son iki kare dönsün (6 ve 7)
      if (menuKaresi == 6)
        menuKaresi = 7;
      else
        menuKaresi = 6;
    }
    sonMenuKareZamani = suAn;
  }
  return menuKaresi;
}

void ekraniGuncelle(unsigned long suAn) {
  if (gosterSesAcmaAnimasyonu) {
    static unsigned long sonKareZamani = 0;
    static int sesKaresi = 1;          // İlk karesi (0) gereksiz
    if (suAn - sonKareZamani >= 300) { // Yavaş animasyon
      sesKaresi++;
      if (sesKaresi > 6)
        sesKaresi = 1; // 7 kare (0-6)
      sonKareZamani = suAn;
    }
    display.clearDisplay();
    display.fillRect(0, 0, 128, 64, WHITE);
    const unsigned char *kare = Sesacma_sprite + (sesKaresi * 1024);
    display.drawBitmap(0, 0, kare, 128, 64, WHITE, BLACK); // Renkler düzeltildi
  } else if (gosterSesKismaAnimasyonu) {
    static unsigned long sonKareZamani = 0;
    static int sesKaresi = 0;          // İlk karesi var, son karesi (5) yok
    if (suAn - sonKareZamani >= 300) { // Yavaş animasyon
      sesKaresi++;
      if (sesKaresi > 4)
        sesKaresi = 0; // 6 kare (0-5), 5 yok -> (0-4)
      sonKareZamani = suAn;
    }
    display.clearDisplay();
    display.fillRect(0, 0, 128, 64, WHITE);
    const unsigned char *kare = seskisma_sprite + (sesKaresi * 1024);
    display.drawBitmap(0, 0, kare, 128, 64, WHITE, BLACK); // Renkler düzeltildi
  }

  if (gosterSesAcmaAnimasyonu || gosterSesKismaAnimasyonu) {
    display.setTextColor(BLACK);
    display.setTextSize(1);
    display.setFont(NULL);
    String sesYazisi = "SES " + String(suAnkiSes);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(sesYazisi, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((128 - w) / 2, 45); // Animasyonun üzerinde
    display.print(sesYazisi);
  }

  // Şarj ibaresi (her zaman en üste sağ köşeye çizilir)
  int pil = getPilYuzdesi();
  
  // Pil Yüzdesi Yazısı
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setFont(&TomThumb); // Adafruit'in en küçük fontu
  
  // TomThumb fontu aşağıdan yukarı (baseline) çizilir, o yüzden y=7 yapıyoruz
  int yaziX = 92; 
  if (pil < 10) yaziX = 102;
  else if (pil < 100) yaziX = 97;
  
  display.setCursor(yaziX, 7); 
  display.print(pil);
  display.print("%");
  
  display.setFont(NULL); // Diğer yazılar bozulmasın diye fontu varsayılana geri döndür

  // Biraz daha daraltılmış (kısaltılmış) pil simgesi
  display.drawRect(112, 2, 12, 6, BLACK);
  display.fillRect(124, 4, 2, 2, BLACK);
  int doluPixel = map(pil, 0, 100, 0, 10);
  if (doluPixel > 0) {
    display.fillRect(113, 3, doluPixel, 4, BLACK);
  }

  display.display();
}

// ==========================================
// MENÜ ÇİZİMİ
// ==========================================
void menuCiz(unsigned long suAn) {
  display.clearDisplay();

  // Arka plan bembeyaz
  display.fillRect(0, 0, 128, 64, WHITE);

  // Menü animasyonu
  int menuKaresi = getMenuKaresi(suAn);
  const unsigned char *kare = menu_sprite + (menuKaresi * 1024);
  // Seçim barının tam üst kısmından başlayacak şekilde y ekseninde yukarı
  // kaydırıldı ve renkler düzeltildi
  display.drawBitmap(0, -14, kare, 128, 64, WHITE, BLACK);

  display.setTextColor(BLACK, WHITE);
  display.setTextSize(1);
  display.setFont(NULL);

  String secenek = "";
  if (seciliMenu >= 0 && seciliMenu <= 3) {
    secenek = String(menuSecenekleri[seciliMenu]);
  } else if (seciliMenu == 4) {
    if (uykuSecim == 0)
      secenek = "UYKU < KAPALI >";
    else
      secenek = "UYKU < " + String(uykuSureDakika[uykuSecim]) + " DK >";
  } else if (seciliMenu == 5) {
    if (geceSecim == 0)
      secenek = "GECE < KAPALI >";
    else
      secenek = "GECE < ACIK >";
  } else if (seciliMenu == 6) {
    secenek = "MAX SES < " + String(sesDegerleri[sesSecim]) + " >";
  }

  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(secenek, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 52);
  display.print(secenek);

  ekraniGuncelle(suAn);
}

void uykuMenuCiz(unsigned long suAn) {
  display.clearDisplay();

  // Arka plan bembeyaz
  display.fillRect(0, 0, 128, 64, WHITE);

  display.setTextColor(BLACK, WHITE);
  display.setTextSize(1);
  display.setFont(NULL);

  String baslik = "--- UYKU MODU ---";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(baslik, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 10);
  display.print(baslik);

  String secenek = uykuSecenekleri[uykuSecim];
  display.getTextBounds(secenek, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 35);

  // Yanıp sönme (Blink) efekti
  if ((suAn % 600) < 300) {
    display.print(secenek);
  }

  ekraniGuncelle(suAn);
}

void geceMenuCiz(unsigned long suAn) {
  display.clearDisplay();

  // Arka plan bembeyaz
  display.fillRect(0, 0, 128, 64, WHITE);

  display.setTextColor(BLACK, WHITE);
  display.setTextSize(1);
  display.setFont(NULL);

  String baslik = "--- GECE MODU ---";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(baslik, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 10);
  display.print(baslik);

  String secenek = geceSecenekleri[geceSecim];
  display.getTextBounds(secenek, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 35);

  // Yanıp sönme (Blink) efekti
  if ((suAn % 600) < 300) {
    display.print(secenek);
  }

  ekraniGuncelle(suAn);
}

void sesMenuCiz(unsigned long suAn) {
  display.clearDisplay();

  // Arka plan bembeyaz
  display.fillRect(0, 0, 128, 64, WHITE);

  display.setTextColor(BLACK, WHITE);
  display.setTextSize(1);
  display.setFont(NULL);

  String baslik = "--- SES SINIRI ---";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(baslik, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 10);
  display.print(baslik);

  String secenek = sesSecenekleri[sesSecim];
  display.getTextBounds(secenek, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 35);

  // Yanıp sönme efekti
  if ((suAn % 600) < 300) {
    display.print(secenek);
  }

  ekraniGuncelle(suAn);
}

// ==========================================
// 4. ANA DÖNGÜ
// ==========================================
void loop() {
  unsigned long suAn = millis();

  // UYKU ZAMANLAYICI KONTROLÜ
  if (uykuHedefSuresi > 0) {
    if (suAn - uykuBaslamaZamani >= uykuHedefSuresi) {
      sistemDurumu = KAPANIS;
      suAnkiKare = 0;
      sonKareZamani = suAn;
      sarkiCaliyorMu = false;
      uykuHedefSuresi = 0; // Bir daha tetiklenmemesi için
    }
  }

  // OTOMATİK ŞARKI GEÇİŞİ (Global - Menüdeyken de çalışsın)
  static unsigned long sonDfSorgu = 0;
  if (suAn - sonDfSorgu > 500) {
    sonDfSorgu = suAn;
    if (myDFPlayer.available()) {
      uint8_t tip = myDFPlayer.readType();
      if (tip == DFPlayerPlayFinished) {
        static unsigned long sonOtomatikGecis = 0;
        if (suAn - sonOtomatikGecis > 2000) {
          suAnkiSahne++;
          if (suAnkiSahne > klasorSarkiSayilari[aktifKategori])
            suAnkiSahne = 1;
          sonSarkilar[aktifKategori] = suAnkiSahne; // Son şarkıyı kaydet
          sarkiBaslat(suAnkiSahne);
          sonOtomatikGecis = suAn;
        }
      }
    }
  }

  switch (sistemDurumu) {
  case GIRIS: {
    animasyonGiris(suAn);
    break;
  }

  case KAPANIS: {
    animasyonKapanis(suAn);
    break;
  }

  case DURUM_MENU:
  case DURUM_AYAR_UYKU:
  case DURUM_AYAR_GECE:
  case DURUM_AYAR_SES:
    butonlariKontrolEt(suAn);
    if (sistemDurumu == DURUM_MENU)
      menuCiz(suAn);
    else if (sistemDurumu == DURUM_AYAR_UYKU)
      uykuMenuCiz(suAn);
    else if (sistemDurumu == DURUM_AYAR_GECE)
      geceMenuCiz(suAn);
    else if (sistemDurumu == DURUM_AYAR_SES)
      sesMenuCiz(suAn);
    break;

  case DURUM_OYNATMA:
    // Butonları kontrol et
    butonlariKontrolEt(suAn);

    // Animasyon döngüsü - Şarkı duraklatıldıysa animasyon süresini de dondur
    static unsigned long gecenAnimasyonSuresi = 0;
    if (sarkiCaliyorMu) {
      if (suAn >= animasyonBaslamaZamani) {
        gecenAnimasyonSuresi = suAn - animasyonBaslamaZamani;
      } else {
        gecenAnimasyonSuresi = 0;
      }
    } else {
      animasyonBaslamaZamani = suAn - gecenAnimasyonSuresi;
    }

    // O an şarkı sözü var mı kontrol et
    bool sozVarMi = false;
    unsigned long sarkiGecenSuresi = 0;
    if (suAn >= sarkiBaslamaMilis) {
      sarkiGecenSuresi = suAn - sarkiBaslamaMilis;
    }
    const SarkiSozu *sozler = getSenkronizeSozler(aktifKategori, suAnkiSahne);
    if (sozler != nullptr) {
      for (int i = 0; sozler[i].metin != nullptr; i++) {
        if (sarkiGecenSuresi >= sozler[i].baslamaZamani &&
            sarkiGecenSuresi <= sozler[i].bitisZamani) {
          sozVarMi = true;
          break;
        }
        if (sarkiGecenSuresi < sozler[i].baslamaZamani &&
            sozler[i].baslamaZamani - sarkiGecenSuresi <= 500) {
          sozVarMi = true;
          break;
        }
      }
    }

    int mevcutAnimTipi = ANIMASYON_SIRASI[suAnkiAnimasyon];
    bool mevcutTamEkran =
        (mevcutAnimTipi == 4 || mevcutAnimTipi == 6 || mevcutAnimTipi == 8 || mevcutAnimTipi >= 9);

    // Eğer tam ekran animasyon oynarken söz geldiyse (veya gelecekse), anında
    // 64x64 bir animasyonla yer değiştir! (Klasikte değişime izin verme)
    if (sozVarMi && mevcutTamEkran && aktifKategori != 3) {
      int anlikYeniAnim = random(0, TOPLAM_ANIMASYON - 2); // 9 ve 10 hariç
      int anlikYeniTip = ANIMASYON_SIRASI[anlikYeniAnim];
      while (anlikYeniTip == 4 || anlikYeniTip == 6 || anlikYeniTip == 8 || anlikYeniTip >= 9) {
        anlikYeniAnim = random(0, TOPLAM_ANIMASYON - 2);
        anlikYeniTip = ANIMASYON_SIRASI[anlikYeniAnim];
      }
      suAnkiAnimasyon = anlikYeniAnim;
      suAnkiKare = 0;
      animasyonBaslamaZamani = suAn;
      gecenAnimasyonSuresi = 0;
    }

    if (gecenAnimasyonSuresi >= ANIMASYON_SURESI[suAnkiAnimasyon]) {
      int eskiAnimTipi = ANIMASYON_SIRASI[suAnkiAnimasyon];
      bool eskiTamEkran =
          (eskiAnimTipi == 4 || eskiAnimTipi == 6 || eskiAnimTipi == 8 ||
           eskiAnimTipi >= 9);

      int yeniAnim;
      int yeniAnimTipi;
      bool yeniTamEkran;

      if (aktifKategori == 3) {
        // KLASİK KATEGORİSİ: 9 (Piyano) ve 10 (Keman) döner
        yeniAnim = random(9, 11);
        while (yeniAnim == suAnkiAnimasyon) {
          yeniAnim = random(9, 11); // Aynı animasyon tekrar gelmesin diye
        }
      } else {
        // DİĞER KATEGORİLER: 9 ve 10 HARİÇ diğerleri rastgele döner
        yeniAnim = random(0, TOPLAM_ANIMASYON - 2); // 0'dan 8'e kadar seçer
        yeniAnimTipi = ANIMASYON_SIRASI[yeniAnim];
        yeniTamEkran =
            (yeniAnimTipi == 4 || yeniAnimTipi == 6 || yeniAnimTipi == 8 ||
             yeniAnimTipi >= 9);

        // Kurallar:
        // 1. Aynı animasyon gelmesin
        // 2. Ard arda iki tam ekran gelmesin
        // 3. Şarkı sözü oynarken tam ekran gelmesin
        while (yeniAnim == suAnkiAnimasyon || (eskiTamEkran && yeniTamEkran) ||
               (sozVarMi && yeniTamEkran)) {
          yeniAnim = random(0, TOPLAM_ANIMASYON - 2); // 9 ve 10 hariç
          yeniAnimTipi = ANIMASYON_SIRASI[yeniAnim];
          yeniTamEkran =
              (yeniAnimTipi == 4 || yeniAnimTipi == 6 || yeniAnimTipi == 8 ||
               yeniAnimTipi >= 9);
        }
      }

      suAnkiAnimasyon = yeniAnim;
      suAnkiKare = 0;
      sonKareZamani = suAn;
      animasyonBaslamaZamani = suAn;
    }

    // Ekranı çiz
    arayuzuCiz(suAn);
    break;
  }
  yield();  // Watchdog Reset'i önle!
  delay(1); // ESP32'ye nefes aldır
}

// ==========================================
// 5. KARE KONTROL (Her animasyon için ayrı hız)
// ==========================================
void kareKontrol(unsigned long suAn, int maxKare, int kareHizi) {
  if (!sarkiCaliyorMu && sistemDurumu == DURUM_OYNATMA)
    return; // Şarkı durduğunda animasyonu da dondur!
  if (suAn - sonKareZamani >= (unsigned long)kareHizi) {
    suAnkiKare++;
    if (suAnkiKare >= maxKare)
      suAnkiKare = 0;
    sonKareZamani = suAn;
  }
}

// ==========================================
// 6. GİRİŞ ANİMASYONU (11 Kare, son kare uzun)
// ==========================================
void animasyonGiris(unsigned long suAn) {
  // Kareleri manuel yönet (son kare farklı hızda)
  if (suAnkiKare < 10) {
    // Normal kareler
    if (suAn - sonKareZamani >= (unsigned long)HIZI_GIRIS) {
      suAnkiKare++;
      sonKareZamani = suAn;
    }
  } else {
    // Son kare (10) - daha uzun bekle, sonra normal moda geç
    if (suAn - sonKareZamani >= (unsigned long)HIZI_GIRIS_SONKARE) {
      myDFPlayer.volume(15);
      suAnkiSes = 15;
      sistemDurumu = DURUM_MENU;
      suAnkiKare = 0;
      sonKareZamani = suAn;
      return;
    }
  }

  // Ekranı çiz
  const unsigned char *kare = giris_sprite + (suAnkiKare * 512);
  display.clearDisplay();
  // Resim Sağda
  display.drawBitmap(64, 0, kare, 64, 64, WHITE);

  // Yazı Alanı Solda (Beyaz zemin siyah yazı)
  display.fillRect(0, 0, 64, 64, WHITE);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setFont(NULL); // Standart okunakli font

  yaziyiKutuyaYaz("Hosgeldin askim!!", 0, 0, 64, 64, suAn, 9999);
  display.display();
}

// ==========================================
// 7. KAPANIŞ ANİMASYONU (7 Kare)
// ==========================================
void animasyonKapanis(unsigned long suAn) {
  // Müziği duraklat (stop yerine pause kullanıyoruz ki uyandığında kaldığı yerden devam edebilsin)
  static bool muzikDurduruldu = false;
  if (!muzikDurduruldu) {
    if (sarkiCaliyorMu) {
      myDFPlayer.pause();
      sarkiDurdurmaMilis = millis();
      sarkiCaliyorMu = false;
      delay(100);
    }
    muzikDurduruldu = true;
  }

  // Kareleri ilerlet
  if (suAnkiKare < 6) {
    if (suAn - sonKareZamani >= (unsigned long)HIZI_KAPANIS) {
      suAnkiKare++;
      sonKareZamani = suAn;
    }
  } else {
    // Son kare - belirli bir süre bekle sonra sistemi durdur
    if (suAn - sonKareZamani >= (unsigned long)HIZI_KAPANIS_SONKARE) {
      display.clearDisplay();
      display.display();
      
      // ESP32-C3 Uyanma Ayarı
      // Pin 7 (Dijital GPIO) Derin Uyku'dan uyanamadığı için Hafif Uyku (Light Sleep) kullanıyoruz.
      // Güç tüketimi farkı çok azdır (0.1 mA).
      gpio_wakeup_enable((gpio_num_t)btnOynat, GPIO_INTR_LOW_LEVEL);
      esp_sleep_enable_gpio_wakeup();

      esp_light_sleep_start(); // Cihazı hafif uykuya al (Kod burada duraklar)

      // === CİHAZ BURADA UYANDI ===
      // Kullanıcı elini butondan çekene kadar bekle ki tekrar menüde basılmış gibi algılamasın
      while(digitalRead(btnOynat) == LOW) {
        delay(10);
      }
      
      muzikDurduruldu = false;
      
      // Uyandığında giriş animasyonuna dönmek yerine direkt müziği başlat
      // ve aynen kaldığı yerden oynatma ekranına geç.
      sistemDurumu = DURUM_OYNATMA;
      sarkiCaliyorMu = true;
      myDFPlayer.start(); // Kaldığı yerden oynatmaya devam et
      
      // Şarkı sözleri senkronizasyonunun bozulmaması için uyku süresini atla
      if (sarkiDurdurmaMilis > 0) {
        sarkiBaslamaMilis += (millis() - sarkiDurdurmaMilis);
        sarkiDurdurmaMilis = 0;
      }
      return;
    }
  }

  // Ekranı çiz
  const unsigned char *kare = kapanis_sprite + (suAnkiKare * 512);
  display.clearDisplay();
  // Resim Solda
  display.drawBitmap(0, 0, kare, 64, 64, WHITE);

  // Yazı Alanı Sağda
  display.fillRect(64, 0, 64, 64, WHITE);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setFont(NULL); // Standart okunakli font

  yaziyiKutuyaYaz("Seni bekleyece-gim...", 64, 0, 64, 64, suAn, 9999);
  display.display();
}

// ==========================================
// 8. ŞARKI ANİMASYONLARI
// ==========================================
void animasyonKedi(unsigned long suAn, int x) {
  kareKontrol(suAn, 4, HIZI_KEDI);
  display.drawBitmap(x, 0, kedi_anim[suAnkiKare], 64, 64, WHITE);
}

void animasyonMuzik(unsigned long suAn, int x) {
  kareKontrol(suAn, 3, HIZI_MUZIK);
  display.drawBitmap(x, 0, muzik_anim[suAnkiKare], 64, 64, WHITE);
}

void animasyonKenarKedi(unsigned long suAn, int x) {
  kareKontrol(suAn, 4, HIZI_KENARKEDI);
  display.drawBitmap(x, 0, kenarKedi_anim[suAnkiKare], 64, 64, WHITE);
}

void animasyonBalik(unsigned long suAn, int x) {
  kareKontrol(suAn, 11, HIZI_BALIK);
  const unsigned char *kare = balik_sprite + (suAnkiKare * 512);
  display.drawBitmap(x, 0, kare, 64, 64, WHITE);
}

void animasyonAsk(unsigned long suAn, int x) {
  // Setup'ta önceden döndürülmüş resmi tek seferde çiz (süper hızlı, reset
  // riski sıfır!)
  display.drawBitmap(x, 0, askRotated, 128, 64, WHITE);
}

void animasyonPlak(unsigned long suAn, int x) {
  kareKontrol(suAn, 9, HIZI_PLAK);
  const unsigned char *kare = plak_sprite + (suAnkiKare * 512);
  display.drawBitmap(x, 0, kare, 64, 64, WHITE);
}

void animasyonKeduslar(unsigned long suAn, int x) {
  // 128x64 yatay tam ekran animasyonu.
  kareKontrol(suAn, 10, HIZI_KEDUSLAR);
  const unsigned char *kare = keduslar_sprite + (suAnkiKare * 1024);
  display.drawBitmap(x, 0, kare, 128, 64, WHITE);
}

void animasyonOpusenCift(unsigned long suAn, int x) {
  // 64x64, 5 kare
  kareKontrol(suAn, 5, HIZI_OPUSEN);
  const unsigned char *kare = opusenCift_sprite + (suAnkiKare * 512);
  display.drawBitmap(x, 0, kare, 64, 64, WHITE);
}

void animasyonSarilanCift(unsigned long suAn, int x) {
  // 128x64 yatay tam ekran animasyonu, 11 kare
  kareKontrol(suAn, 11, HIZI_SARILAN);
  const unsigned char *kare = sarilanCift_sprite + (suAnkiKare * 1024);
  display.drawBitmap(x, 0, kare, 128, 64, WHITE);
}

void animasyonPiano(unsigned long suAn, int x) {
  // 128x128px sprite aslında alt alta 2 adet 128x64 kare içerir (Toplam 2 kare)
  kareKontrol(suAn, 2, HIZI_PIANO);
  const unsigned char *kare = piano_sprite + (suAnkiKare * 1024);
  display.drawBitmap(x, 0, kare, 128, 64, WHITE);
}

void animasyonKeman(unsigned long suAn, int x) {
  // 128x384px sprite -> 6 kare (128x64)
  kareKontrol(suAn, 6, HIZI_KEMAN);
  const unsigned char *kare = keman_sprite + (suAnkiKare * 1024);
  display.drawBitmap(x, 0, kare, 128, 64, WHITE);
}

// ==========================================
// TÜRKÇE KARAKTER TEMİZLEYİCİ
// ==========================================
String turkceKarakterTemizle(String metin) {
  metin.replace("ş", "s");
  metin.replace("Ş", "S");
  metin.replace("ı", "i");
  metin.replace("İ", "I");
  metin.replace("ğ", "g");
  metin.replace("Ğ", "G");
  metin.replace("ü", "u");
  metin.replace("Ü", "U");
  metin.replace("ö", "o");
  metin.replace("Ö", "O");
  metin.replace("ç", "c");
  metin.replace("Ç", "C");
  return metin;
}

// ==========================================
// 8.5 KUTU İÇİNE YAZI YAZDIRMA (Nizami, Ortalı ve Kelime Bölmeden)
// ==========================================
void yaziyiKutuyaYaz(const char *metin_raw, int kutuX, int kutuY, int kutuW,
                     int kutuH, unsigned long suAn, int harfLimiti) {
  String temizMetin = turkceKarakterTemizle(String(metin_raw));
  const char *metin = temizMetin.c_str();

  int satirYuksekligi = 10; // Standart Font için
  String satirlar[30];
  int satirSayisi = 0;

  String aktifSatir = "";
  String aktifKelime = "";

  for (int i = 0; i <= strlen(metin); i++) {
    char c = metin[i];

    if (c == ' ' || c == '\n' || c == '\0') {
      // Kelime bitti.
      String testSatir = aktifSatir;
      if (testSatir.length() > 0 && c != '\n')
        testSatir += " ";
      testSatir += aktifKelime;

      int16_t x1, y1;
      uint16_t w, h;
      display.getTextBounds(testSatir, 0, 0, &x1, &y1, &w, &h);

      if (w > (kutuW - 4)) {
        // Eğer kelime tek başına satıra sığmıyorsa harf harf bölmeliyiz!
        display.getTextBounds(aktifKelime, 0, 0, &x1, &y1, &w, &h);
        if (w > (kutuW - 4)) {
          // Kelime çok uzun! Harf harf bölelim.
          if (aktifSatir.length() > 0) {
            satirlar[satirSayisi++] = aktifSatir;
            aktifSatir = "";
          }
          String parca = "";
          for (int k = 0; k < aktifKelime.length(); k++) {
            String testParca = parca + aktifKelime[k];
            display.getTextBounds(testParca, 0, 0, &x1, &y1, &w, &h);
            if (w > (kutuW - 4)) {
              satirlar[satirSayisi++] = parca;
              parca = String(aktifKelime[k]);
            } else {
              parca = testParca;
            }
          }
          aktifSatir = parca; // Kalan parça yeni aktif satır olur
        } else {
          // Kelime normal uzunlukta ama bu satıra sığmadı.
          if (aktifSatir.length() > 0) {
            satirlar[satirSayisi++] = aktifSatir;
          }
          aktifSatir = aktifKelime;
        }
      } else {
        // Sorunsuz sığıyor
        aktifSatir = testSatir;
      }

      if (c == '\n') {
        satirlar[satirSayisi++] = aktifSatir;
        aktifSatir = "";
      }
      aktifKelime = "";

    } else {
      aktifKelime += c;
    }
  }

  if (aktifSatir.length() > 0) {
    satirlar[satirSayisi++] = aktifSatir;
  }

  // Dikey Ortalama
  int toplamYaziBoyu = satirSayisi * satirYuksekligi;
  int baslangicY;

  if (toplamYaziBoyu > kutuH) {
    baslangicY = kutuY + 2;
  } else {
    baslangicY = kutuY + ((kutuH - toplamYaziBoyu) / 2);
  }

  int harfSayaci = 0;

  // Çizdir
  for (int i = 0; i < satirSayisi; i++) {
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(satirlar[i], 0, 0, &x1, &y1, &w, &h);

    int satirX = kutuX + ((kutuW - w) / 2);
    int satirY = baslangicY + (i * satirYuksekligi);

    String cizilecekSatir = "";
    for (int k = 0; k < satirlar[i].length(); k++) {
      if (harfSayaci < harfLimiti) {
        cizilecekSatir += satirlar[i][k];
        harfSayaci++;
      } else {
        break;
      }
    }

    if (satirY >= kutuY - 8 &&
        satirY <= kutuY + kutuH) { // Standart font clipping
      if (cizilecekSatir.length() > 0) {
        display.setCursor(satirX, satirY);
        display.print(cizilecekSatir);
      }
    }

    if (harfSayaci >= harfLimiti) {
      break; // Harf limitine ulaşıldı, diğer satırları çizmeye gerek yok
    }
  }
}

// ==========================================
// 9. ARAYÜZ ÇİZİMİ
// ==========================================
void arayuzuCiz(unsigned long suAn) {
  display.clearDisplay();

  int animTipi = ANIMASYON_SIRASI[suAnkiAnimasyon];
  bool tamEkran = (animTipi == 4 || animTipi == 6 || animTipi == 8 || animTipi >= 9);

  // --- YAZI ALANI HESAPLAMA ---
  unsigned long gecenSarkiSuresi = suAn - sarkiBaslamaMilis;
  String yaziGosterilecek = "";
  bool ozelSozBulundu = false;

  if (!sarkiCaliyorMu && sarkiDurdurmaMilis > 0) {
    gecenSarkiSuresi = sarkiDurdurmaMilis - sarkiBaslamaMilis;
  }

  int harfLimiti = 9999;
  unsigned long aktifSozBaslama = 0;
  bool sozGeliyor = false;
  unsigned long sonSozBitisZamani = 0;

  const SarkiSozu *sozler = getSenkronizeSozler(aktifKategori, suAnkiSahne);
  if (sozler != nullptr) {
    for (int i = 0; sozler[i].metin != nullptr; i++) {
      if (gecenSarkiSuresi >= sozler[i].baslamaZamani &&
          gecenSarkiSuresi <= sozler[i].bitisZamani) {
        yaziGosterilecek = sozler[i].metin;
        ozelSozBulundu = true;
        aktifSozBaslama = sozler[i].baslamaZamani;
      } else if (gecenSarkiSuresi < sozler[i].baslamaZamani &&
                 sozler[i].baslamaZamani - gecenSarkiSuresi <= 500) {
        sozGeliyor = true;
      }

      if (gecenSarkiSuresi > sozler[i].bitisZamani) {
        if (sozler[i].bitisZamani > sonSozBitisZamani) {
          sonSozBitisZamani = sozler[i].bitisZamani;
        }
      }
    }
  }

  if (sarkiCaliyorMu) {
    if (ozelSozBulundu) {
      unsigned long aktifSozGecen = gecenSarkiSuresi - aktifSozBaslama;
      harfLimiti = aktifSozGecen / 60;
      tamEkran = false; 
    } else if (sozGeliyor) {
      harfLimiti = 0;
      tamEkran = false; 
    } else {
      unsigned long yazmayaBaslamaSuresi = gecenSarkiSuresi - sonSozBitisZamani;
      harfLimiti = yazmayaBaslamaSuresi / 40; 
    }
  }

  if (!ozelSozBulundu) {
    if (aktifKategori >= 0 && aktifKategori < 4) {
      if (suAnkiSahne >= 1 && suAnkiSahne <= sarkiYazilari[aktifKategori].size()) {
        yaziGosterilecek = sarkiYazilari[aktifKategori][suAnkiSahne - 1];
      } else {
        // Kullanıcının koda manuel metin eklemediği şarkılarda ekran boş kalacak
        yaziGosterilecek = "";
      }
    }
  }

  bool yaziCizilecek = (!tamEkran && (yaziGosterilecek.length() > 0 || !sarkiCaliyorMu || sozGeliyor));
  bool yazisizMod = (!tamEkran && !yaziCizilecek);

  // --- ANİMASYON ÇİZİMİ ---
  int resimX = 0;
  int yaziKutuX = 0;
  int yaziKutuW = 64;

  switch (animTipi) {
  case 0: resimX = 64; yaziKutuX = 0; break;
  case 1: resimX = 0; yaziKutuX = 64; break;
  case 2: resimX = 0; yaziKutuX = 64; break;
  case 3: resimX = 64; yaziKutuX = 0; break;
  case 4: tamEkran = true; yaziKutuX = 64; break;
  case 5: resimX = 0; yaziKutuX = 64; break;
  case 6: tamEkran = true; yaziKutuX = 0; break;
  case 7: resimX = 64; yaziKutuX = 0; break;
  case 8: tamEkran = true; yaziKutuX = 64; break;
  case 9: tamEkran = true; yaziKutuX = 64; break;
  case 10: tamEkran = true; yaziKutuX = 64; break;
  default: resimX = 64; yaziKutuX = 0; break;
  }

  if (yazisizMod) {
    resimX = 32; // Ortala
    // Kenarları beyaz yap (siyah animasyon ortada kalır)
    display.fillRect(0, 0, 32, 64, WHITE);
    display.fillRect(96, 0, 32, 64, WHITE);
  }

  // Animasyonu ilgili X koordinatında çiz
  switch (animTipi) {
  case 0: animasyonKedi(suAn, resimX); break;
  case 1: animasyonMuzik(suAn, resimX); break;
  case 2: animasyonKenarKedi(suAn, resimX); break;
  case 3: animasyonBalik(suAn, resimX); break;
  case 4: animasyonAsk(suAn, 0); break;
  case 5: animasyonPlak(suAn, resimX); break;
  case 6: animasyonKeduslar(suAn, 0); break;
  case 7: animasyonOpusenCift(suAn, resimX); break;
  case 8: animasyonSarilanCift(suAn, 0); break;
  case 9: animasyonPiano(suAn, 0); break;
  case 10: animasyonKeman(suAn, 0); break;
  default: animasyonKedi(suAn, resimX); break;
  }

  // --- YAZI KUTUSU ÇİZİMİ ---
  if (yaziCizilecek) {
    int yaziKutuY = 0;
    int yaziKutuH = 64;

    display.fillRect(yaziKutuX, yaziKutuY, yaziKutuW, yaziKutuH, WHITE);
    display.setTextColor(BLACK);
    display.setTextSize(1);
    display.setFont(NULL);

    if (yaziGosterilecek.length() > 0) {
      yaziyiKutuyaYaz(yaziGosterilecek.c_str(), yaziKutuX, yaziKutuY, yaziKutuW,
                      yaziKutuH, suAn, harfLimiti);
    }

    if (!sarkiCaliyorMu) {
      int16_t x1, y1;
      uint16_t w, h;
      display.getTextBounds("(Durduruldu)", 0, 0, &x1, &y1, &w, &h);
      display.setCursor(yaziKutuX + ((yaziKutuW - w) / 2), 54);
      display.print("(Durduruldu)");
    }
  }

  ekraniGuncelle(suAn);
}

// ==========================================
// 10. BUTON KONTROLLERİ
// ==========================================
void butonlariKontrolEt(unsigned long suAn) {
  static unsigned long dbIleri = 0, dbGeri = 0, dbOynat = 0;
  static bool sonIleri = HIGH, sonGeri = HIGH, sonOynat = HIGH;
  static bool gecIleri = HIGH, gecGeri = HIGH, gecOynat = HIGH;

  bool okuIleri = digitalRead(btnIleri);
  bool okuGeri = digitalRead(btnGeri);
  bool okuOynat = digitalRead(btnOynat);

  // Debounce Mantığı
  if (okuIleri != sonIleri)
    dbIleri = suAn;
  if (okuGeri != sonGeri)
    dbGeri = suAn;
  if (okuOynat != sonOynat)
    dbOynat = suAn;

  // İleri Butonu İşleme
  if ((suAn - dbIleri) > 50 && okuIleri != gecIleri) {
    gecIleri = okuIleri;
    if (gecIleri == LOW) {
      btnIleriZaman = suAn;
      btnIleriBasili = true;
      btnIleriUzun = false;
    } else {
      btnIleriBasili = false;
      if (!btnIleriUzun) {
        // KISA BASIM İŞLEVLERİ
        if (sistemDurumu == DURUM_MENU) {
          seciliMenu++;
          if (seciliMenu > 6)
            seciliMenu = 0;
        } else if (sistemDurumu == DURUM_AYAR_UYKU) {
          uykuSecim++;
          if (uykuSecim > 4)
            uykuSecim = 0;
        } else if (sistemDurumu == DURUM_AYAR_GECE) {
          geceSecim++;
          if (geceSecim > 1)
            geceSecim = 0;
        } else if (sistemDurumu == DURUM_AYAR_SES) {
          sesSecim++;
          if (sesSecim > 4)
            sesSecim = 0;
        } else if (sistemDurumu == DURUM_OYNATMA) {
          suAnkiSahne++;
          if (suAnkiSahne > klasorSarkiSayilari[aktifKategori])
            suAnkiSahne = 1;
          sonSarkilar[aktifKategori] = suAnkiSahne;
          sarkiBaslat(suAnkiSahne);
        }
      }
    }
  }

  // Geri Butonu İşleme
  if ((suAn - dbGeri) > 50 && okuGeri != gecGeri) {
    gecGeri = okuGeri;
    if (gecGeri == LOW) {
      btnGeriZaman = suAn;
      btnGeriBasili = true;
      btnGeriUzun = false;
    } else {
      btnGeriBasili = false;
      if (!btnGeriUzun) {
        // KISA BASIM İŞLEVLERİ
        if (sistemDurumu == DURUM_MENU) {
          seciliMenu--;
          if (seciliMenu < 0)
            seciliMenu = 6;
        } else if (sistemDurumu == DURUM_AYAR_UYKU) {
          uykuSecim--;
          if (uykuSecim < 0)
            uykuSecim = 4;
        } else if (sistemDurumu == DURUM_AYAR_GECE) {
          geceSecim--;
          if (geceSecim < 0)
            geceSecim = 1;
        } else if (sistemDurumu == DURUM_AYAR_SES) {
          sesSecim--;
          if (sesSecim < 0)
            sesSecim = 4;
        } else if (sistemDurumu == DURUM_OYNATMA) {
          suAnkiSahne--;
          if (suAnkiSahne < 1)
            suAnkiSahne = klasorSarkiSayilari[aktifKategori];
          sonSarkilar[aktifKategori] = suAnkiSahne;
          sarkiBaslat(suAnkiSahne);
        }
      }
    }
  }

  // Orta Buton İşleme
  if ((suAn - dbOynat) > 50 && okuOynat != gecOynat) {
    gecOynat = okuOynat;
    if (gecOynat == LOW) {
      btnOynatZaman = suAn;
      btnOynatBasili = true;
      btnOynatUzun = false;
    } else {
      btnOynatBasili = false;
      if (!btnOynatUzun) {
        // KISA BASIM İŞLEVLERİ
        if (sistemDurumu == DURUM_MENU) {
          // Görsel Onay (Seçilince Tümü Sönsün)
          display.clearDisplay();
          display.fillRect(0, 0, 128, 64, WHITE);
          display.display();
          delay(300);

          if (seciliMenu >= 0 && seciliMenu <= 3) {
            if (aktifKategori == seciliMenu && sarkiBaslamaMilis > 0) {
              // Aynı klasöre tekrar tıklandığında baştan başlatma, direkt geçiş
              // yap!
              sistemDurumu = DURUM_OYNATMA;
            } else {
              // Farklı müzik modu seçildi veya ilk oynatım
              aktifKategori = seciliMenu;
              
              suAnkiSahne = sonSarkilar[aktifKategori];
              sistemDurumu = DURUM_OYNATMA;
              sarkiBaslat(suAnkiSahne);

              // Kategori değiştiğinde animasyonu anında o kategoriye uygun
              // ayarla
              if (aktifKategori == 3) {
                suAnkiAnimasyon = random(9, 11); // Klasik seçilince direkt piyano veya kemana geç
              } else if (suAnkiAnimasyon >= 9) {
                suAnkiAnimasyon =
                    random(0, TOPLAM_ANIMASYON - 2); // Piyano veya kemandan çık (0-8 arası)
              }
            }
          } else if (seciliMenu == 4) {
            sistemDurumu = DURUM_AYAR_UYKU;
          } else if (seciliMenu == 5) {
            sistemDurumu = DURUM_AYAR_GECE;
          } else if (seciliMenu == 6) {
            sistemDurumu = DURUM_AYAR_SES;
          }
        } else if (sistemDurumu == DURUM_OYNATMA) {
          if (sarkiCaliyorMu) {
            myDFPlayer.pause();
            sarkiDurdurmaMilis = millis();
          } else {
            myDFPlayer.start();
            if (sarkiDurdurmaMilis > 0) {
              sarkiBaslamaMilis += (millis() - sarkiDurdurmaMilis);
              sarkiDurdurmaMilis = 0;
            }
          }
          sarkiCaliyorMu = !sarkiCaliyorMu;
        } else if (sistemDurumu == DURUM_AYAR_UYKU) {
          // Görsel Onay (Seçilince Tamamen Yansın)
          display.clearDisplay();
          display.fillRect(0, 0, 128, 64, WHITE);
          display.setTextColor(BLACK);
          display.setTextSize(1);
          display.setFont(NULL);

          String baslik = "--- UYKU MODU ---";
          int16_t x1, y1;
          uint16_t w, h;
          display.getTextBounds(baslik, 0, 0, &x1, &y1, &w, &h);
          display.setCursor((128 - w) / 2, 10);
          display.print(baslik);

          String secenek = uykuSecenekleri[uykuSecim];
          display.getTextBounds(secenek, 0, 0, &x1, &y1, &w, &h);
          display.setCursor((128 - w) / 2, 35);
          display.print(secenek); // Sabit yazdır
          display.display();
          delay(400);

          uykuHedefSuresi =
              (unsigned long)uykuSureDakika[uykuSecim] * 60UL * 1000UL;
          if (uykuHedefSuresi > 0)
            uykuBaslamaZamani = millis();
          sistemDurumu = DURUM_MENU;
        } else if (sistemDurumu == DURUM_AYAR_GECE) {
          // Görsel Onay (Seçilince Tamamen Yansın)
          display.clearDisplay();
          display.fillRect(0, 0, 128, 64, WHITE);
          display.setTextColor(BLACK);
          display.setTextSize(1);
          display.setFont(NULL);

          String baslik = "--- GECE MODU ---";
          int16_t x1, y1;
          uint16_t w, h;
          display.getTextBounds(baslik, 0, 0, &x1, &y1, &w, &h);
          display.setCursor((128 - w) / 2, 10);
          display.print(baslik);

          String secenek = geceSecenekleri[geceSecim];
          display.getTextBounds(secenek, 0, 0, &x1, &y1, &w, &h);
          display.setCursor((128 - w) / 2, 35);
          display.print(secenek); // Sabit yazdır
          display.display();
          delay(400);

          if (geceSecim == 1) {
            display.dim(true); // Loş mod
          } else {
            display.dim(false); // Normal mod
          }
          sistemDurumu = DURUM_MENU;
        } else if (sistemDurumu == DURUM_AYAR_SES) {
          // Görsel Onay (Seçilince Tamamen Yansın)
          display.clearDisplay();
          display.fillRect(0, 0, 128, 64, WHITE);
          display.setTextColor(BLACK);
          display.setTextSize(1);
          display.setFont(NULL);

          String baslik = "--- MAX SES SINIRI ---";
          int16_t x1, y1;
          uint16_t w, h;
          display.getTextBounds(baslik, 0, 0, &x1, &y1, &w, &h);
          display.setCursor((128 - w) / 2, 10);
          display.print(baslik);

          String secenek = sesSecenekleri[sesSecim];
          display.getTextBounds(secenek, 0, 0, &x1, &y1, &w, &h);
          display.setCursor((128 - w) / 2, 35);
          display.print(secenek); // Sabit yazdır
          display.display();
          delay(400);

          MAX_SES = sesDegerleri[sesSecim]; // Yeni max ses sınırını uygula
          if (suAnkiSes >
              MAX_SES) { // Eğer mevcut ses yeni sınırdan büyükse düşür
            suAnkiSes = MAX_SES;
            myDFPlayer.volume(suAnkiSes);
          }
          sistemDurumu = DURUM_MENU;
        }
      }
    }
  }

  sonIleri = okuIleri;
  sonGeri = okuGeri;
  sonOynat = okuOynat;

  // UZUN BASIM İŞLEVLERİ
  gosterSesAcmaAnimasyonu =
      (gecIleri == LOW && btnIleriBasili && (suAn - btnIleriZaman > 600));
  if (gosterSesAcmaAnimasyonu) {
    btnIleriUzun = true;
    static unsigned long sonSesArtirma = 0;
    if (suAn - sonSesArtirma > 300) {
      if (suAnkiSes < MAX_SES) {
        suAnkiSes++;
        myDFPlayer.volume(suAnkiSes);
        sesGosterimZamani = suAn;
      }
      sonSesArtirma = suAn;
    }
  }

  gosterSesKismaAnimasyonu =
      (gecGeri == LOW && btnGeriBasili && (suAn - btnGeriZaman > 600));
  if (gosterSesKismaAnimasyonu) {
    btnGeriUzun = true;
    static unsigned long sonSesKisma = 0;
    if (suAn - sonSesKisma > 300) {
      if (suAnkiSes > 0) {
        suAnkiSes--;
        myDFPlayer.volume(suAnkiSes);
        sesGosterimZamani = suAn;
      }
      sonSesKisma = suAn;
    }
  }

  if (gecOynat == LOW && btnOynatBasili && (suAn - btnOynatZaman > 1500)) {
    if (!btnOynatUzun) {
      btnOynatUzun = true;
      if (sistemDurumu == DURUM_OYNATMA) {
        // Müzik çalmaya devam etsin, sadece menüye dön
        sistemDurumu = DURUM_MENU;
      } else if (sistemDurumu == DURUM_MENU) {
        sistemDurumu = KAPANIS;
        suAnkiKare = 0;
        sonKareZamani = suAn;
      }
    }
  }
}