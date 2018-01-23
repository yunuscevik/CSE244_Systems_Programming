Usage: ./grepSh 'string'  <directoryName>

Terminalden make ile olusturdugumuz dosyayi calistirirken usage a uygun sekilde 
calistirmamiz gereklidir.
Example:  ./grepSh adana dirA   gibi yazildigi taktirde calismaktadir.
make clean yapildigi taktirde "rm *.o grepSh log.txt" calisip bu dosyalar silinecektir.


Program calistirilip birakildiginda ScreenShot alinmis image1.jpg dekine benzer bir cikti elde edilebilir.
Ayrica terminalde gösterilen bilgilerden "Number of cascade threads created: " haricindekiler log.log dosyam icinde de mevcuttur.
Not: "Number of cascade threads created: " bu ifade de tum directorylerin altinda calisan Threads sayisi olarak anladigimdan
ekrana bir path gibi ("Number of cascade threads created: dirA/dirB/dirC/  : XX threads") bastirmaktayim.

Program calistirilip birkac saniye sonra CTRL + C yapildiği taktirde image2.jpg veya image3.jpg dekine benzer bir çikti elde edilebilir.
image2.jpg deki, olmasi gereken ciktilar ayrica log.txt dosyasinda da duzgun bir sekilde vardir. Ancak bazen image3.jpg deki gibi printf lerden dolayi program CTRL + C ile sonlansa bile printfler recursive fonksiyondan dolayi ("Number of cascade threads created: dirA/dirB/dirC/  : XX threads") sonuc kisminin icine karistirabiliyor. Fakat log.txt dosyasinda duzgun bir sekilde gozukmektedir.

Ayrica Erkan hocanin belirtigi gibi toplam Shared Memory ve her directory altinda bulunan Shared Memorylerin Size lari ekran ciktisinda ve ekstra log.txt dosyasında belirtilmistir. Shared Memorylerin Sizeleri alinirken her dosyanin sizeof(int) ile carpilmisi kadar Shared Memory icin size verdim. Bazi arkadaslar sadece sizeof(int) verip yapmislar ciktilarinda Shared Memory Sizeleri degismiyor hep 4,4,4,4 gibi yaziyor.
Ben bu sekil anladim ve yaptim. Terminale ipcs yazildiginda Message Queue leri ve Shared Memoryleri program calistigi taktirde gorebilirsiniz. Program bir sekilde sonlandiginda ise detach ve remove islemleri yapildigindan ipcs icinde kalinti kalmamaktadir.

Program make ile derlendikten sonra yanlis directory name (./grepSh adana dirAASD) verildigi taktirde image4.jpg gibi bir cikti alabiliriz.
image4.jpg de alt kisimda "Failed to receive message queue: Identifier removed" yazisi ise ana process in isDirectoryOrFile fonksiyonundan(recursive fonksiyon) bilgi almak icin beklmesinden kaynaklidir. Ancak programa olmayan bir directory gonderildiginde isDirectoryOrFile fonksiyonu olmayan dosya path ine giremediginden dogal olarak message queue ile de message gonderilemiyor. Ana process teki message queue silinse bile msqrcv message alamadigindan hata mesaji veriyor. Bu olay programi hicbir turlu etkilememektedir.

Max # of threads running concurrently: bu kisimda directorylerin altindaki threadlerin toplami en yuksek olani bastirdim.

