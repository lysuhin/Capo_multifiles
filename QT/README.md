#Android+QT+OpenCV

Все, что удалось на данный момент. В папке **anroid-tutorial** лежит готовый проект с подключенным OpenCV (чтобы скомпилировался, нужно поменять пути к опенсивишным библиотекам на те, что на локальной машине в файле \*.pro; нужно скачать OpenCV для андроида)

##В общем о QT:

* **%PROJECTNAME%.pro** : Файл с описанием проекта QT. С него все начинается, в нем прописаны названия и пути к исходникам. Также в нем указаны пути к библиотекам и те модули QT (внутренние библиотеки), которые необходимо подключить

* **main.cpp** : главный сишный файл кьютешного приложения

* **main.qml** : главный файл, в котором описывается графический интерфейс приложения на языке QML


##QT Examples:

Прямо внутри QT Creator расположена очень большая коллекция готовых примеров. На стартовом экране нужно нажать кнопку "Tutorials". Исходные файлы этих примеров находятся в директории Qt5.7.0/Examples/Qt-5.7/\*

Пример с камерой, который я показывал, и который планировал взять за основу:
Qt5.7.0/Examples/Qt-5.7/multimedia/declarative-camera

##Кроме основной документации QT:

http://amin-ahmadi.com/ - хороший блог, где можно найти много примеров одновременно по QT, OpenCV и Android

