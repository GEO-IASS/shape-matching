# shape-matching
Реализация поиска в базе изображений по форме с использованием гистограммы по углам касательных для граничных точек.

## Зависимости
gtk3 >= 3.14, glib2, opencv2, autotools

## Компиляция и запуск
``` autoreconf --install --force && ./configure && make && src/shape-matching ```
