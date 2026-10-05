# 16-Bit x86 Hobby Kernel & OS

Bu proje, C ve Assembly dilleri kullanılarak geliştirilmiş 16-bit x86 mimarisiyle çalışan bir hobi işletim sistemi kernel projesidir.

## 🚀 Özellikler

* Custom 16-bit Assembly bootloader (`boot.asm`)
* Real mode uyumlu C kernel entegrasyonu (`kernel.c`, `user.c`)
* Özel bellek düzeni ve bağlama mimarisi (`linker.ld`)
* QEMU sanal makine desteği

## ℹ️ Bilgilendirme & Sorumluluk Reddi

> **Not:** Bu proje, low-level sistem mimarileri, x86 assembly ve C dili ile işletim sistemi çekirdeği (kernel) geliştirme süreçlerini incelemek amacıyla geliştirilmiş bir **eğitim ve hobi projesidir**. 
> 
> BILSIM OS, henüz geliştirme aşamasındadır (WIP). Gerçek sistemlerde ana işletim sistemi olarak kullanılması önerilmez; QEMU, VirtualBox veya benzeri sanal makine ortamlarında çalıştırılması tavsiye edilir.

### 📜 Lisans & Telif Hakkı
© 2026 **Ahmet Kurucuk**. Tüm hakları saklıdır.  
Geliştirici: **Ahmet Kurucuk**

> **Note:** This is an **educational and hobby project** developed to explore low-level system architectures, x86 assembly, and the processes of operating system (kernel) development using the C language.
>
> BILSIM OS is currently a work-in-progress (WIP). It is not recommended for use as a primary operating system on real hardware; running it within virtual machine environments such as QEMU, VirtualBox, or similar is advised.

### 📜 License & Copyright
© 2026 **Developer Ahmet**. All rights reserved.
Developer: **Developer Ahmet (Ahmet Kurucuk)**