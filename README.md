# KryptoCourseMIPT

Для запуска проекта нужно в терминале последовательно прописать:
1) ./sha3gen_build.sh
2) ./avalanche_effect_build.sh
3) ./run_avalanche.sh

Тогда запустится процесс создания графиков зависимости итераций от процента измеменных бит выходной последовательности после изменения одного бита 
во входной последовательности.


Струкрура проекта следующая:
1) Файлы с sha3 содержат реализацию модифицированного алгоритма Keccak
2) Код avalanche_input.py генерирует пары строк, отличающихся на один бит, которые записываются в файл avalanche_text.txt
3) avalanche_text.txt обрататывается программой avalanche_test.out и получаются данные для графика, которые строятся с помощью plot.py

Описание сущности проекта проведено в эссе, посвященном алгоритму Keccak
