Usage: ./grepTh 'string'  <directoryName>

Terminalden make ile olusturdugumuz dosyayi calistirirken usage a uygun sekilde 
calistirmamiz gereklidir.
Example:  ./grepTh adana dirA   gibi yazildigi taktirde calismaktadir.
make clean yapildigi taktirde "rm *.o grepTh *.log" calisip bu dosyalar silinecektir.


Program calistirilip birakildiginda ScreenShot alinmis image1.jpg dekine benzer bir cikti elde edilebilir.
Ayrica terminalde gösterilen bilgilerden "Number of cascade threads created: " haricindekiler log.log dosyam icinde de mevcuttur.
Not: "Number of cascade threads created: " bu ifade de tum directorylerin altinda calisan Threads sayisi olarak anladigimdan
ekrana bir path gibi ("Number of cascade threads created: dirA/dirB/dirC/  : XX threads") bastirmaktayim.

Program calistirilip 1 2 sn sonra CTRL + C yapildiği taktirde image2.jpg veya image3.jpg dekine benzer bir çikti elde edilebilir.
image2.jpg de olan olmasi gereken ciktir ve .log dosyasinda da duzgun bir sekilde vardir. Ancak bazen image3.jpg deki gibi printf lerden dolayi program CTRL + C ile sonlansa bile printfler recursive fonksiyondan dolayi ("Number of cascade threads created: dirA/dirB/dirC/  : XX threads") sonuc kisminin icine karistirabiliyor.Fakat .log dosyasinda duzgun bir sekilde gozukmektedir.


Program make ile derlendikten sonra yanlis directory name (./grepTh adana dirAASD) verildigi taktirde image4.jpg gibi bir cikti alabiliriz.

Max # of threads running concurrently: bu kisimda directorylerin altindaki threadlerin toplami en yuksek olani bastirdim.

