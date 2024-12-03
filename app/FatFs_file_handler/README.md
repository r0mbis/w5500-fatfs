## ˵��
1. Реализует набор часто используемых файловых операций API;
2. Это API поддерживает операции на нескольких дисках;
3. используется по умолчанию (https://github.com/ShadowThree/dbger) и вы можете использовать свой собственный интерфейс вывода журнала в заголовочном файле

## ʹ��
1. ͨДобавьте этот модуль с помощью`git submodule add https://github.com/ShadowThree/fatfs_file_handler ThirdUtils/fatfs_file_handler`ָкоманды;
2. Импортировать файлы заголовков и исходные файлы
3. Определите переменные носителя данных:
```c
// ���µ� xxxPath/xxxFatFS/xxxFile �����Ѿ��� fatfs.c �ж������
StoreDisk_t sDisk[_VOLUMES] = {
    // path       FATFS       FIL        opt     mCnt
    {SDPath, 	&SDFatFS, 	&SDFile, 	FM_FAT32, 0},		// SDcard disk
    {USERPath, 	&USERFatFS, &USERFile, 	FM_FAT,   0}		// SpiFlash disk
};
```
4. атем обратитесь к `FH_API_test()`функции для выполнения операций чтения и записи на указанном диске;