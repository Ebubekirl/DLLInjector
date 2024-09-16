# DLL Enjektörü

Bu proje, Windows üzerinde basit bir DLL enjektörü örneğidir. Bir hedef işlemin belleğini manipüle ederek ve uzaktan thread kullanarak DLL enjekte etme işlemini gösterir. Ayrıca, temel hata yönetimi ve işlem doğrulama işlemlerini içerir.

## Özellikler

- Çalışılan dizindeki mevcut DLL dosyalarını listeler.
- Seçilen DLL dosyasını, belirtilen işlem kimliği (PID) ile hedef işleme enjekte eder.
- Enjekte edilen DLL'nin hedef işlemde yüklü olup olmadığını kontrol eder.
- Bellek tahsisi, uzaktan thread oluşturma ve işlem etkileşimi için temel hata yönetimi uygular.

## Hata Yönetimi

Program, aşağıdaki durumlarda hata mesajları görüntüler:
- Hedef işlem açılamazsa
- Bellek tahsisi başarısız olursa
- Belleğe veri yazılamazsa
- Uzaktan thread oluşturma işlemi başarısız olursa
