# ColorizeTOP
Touchdesigner Colorizing TOP. Opencv Dnn colorizing
"BW" | Color
---- | ----
![Cabane](https://github.com/shieman/ColorizeTOP/blob/master/Images/Cabane.jpg) | ![Cabane Couleur](https://github.com/shieman/ColorizeTOP/blob/master/Images/cabane_couleur.jpg)
![Paulilles](https://github.com/shieman/ColorizeTOP/blob/master/Images/Paulliles.jpg) | ![Paulilles Couleur](https://github.com/shieman/ColorizeTOP/blob/master/Images/Paulilles_couleur.jpg)

# How it works
This c++ Top is made with opencv 4.2 dnn using Cuda 10.2.
It's a Windows version, but you can compile with other os.
Using movie files, i get an average 20fps with cuda dnn, 2 fps without.

# What you need
To make it working you need :
* opencv_world420.dll near Colorizing.dll
  You can get it on https://jamesbowley.co.uk/downloads/
  Take the opencv 4.2 version with cuda 10.2
  In subdirectory x64\vc16\bin, you'll find opencv420.dll
* Cuda 10.2 and cuDNN 7.6.5.32 installed if you want dnn gpu acceleration.
* the machine learning models near colorizing.dll :
    - colorization_deploy_v2.prototxt
    - colorization_release_v2.caffemodel
