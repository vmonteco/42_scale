# 42_scale

A small software to create and edit 42 scale files easily.

![Img](./42_scale.gif)

## Install
#### Dependencies
##### Linux
```
GLFW
GLEW
```
###### Debian
```
# apt-get install libglfw3-dev libglew-dev
```
##### OSX
```
GLFW
GLEW
libyaml
```
###### Brew
```
brew tap homebrew/versions
brew install glew glfw3 libyaml
```
#### Instructions
```
git clone https://github.com/Ne02ptzero/42_scale
```
##### Linux
```
cd libs/libyaml && ./boostrap && ./configure && make
```
#### Compilation
```
make
```

## Execution
```
42_scale ./file.yml
```
