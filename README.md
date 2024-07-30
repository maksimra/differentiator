# Differentiator
## Description
While working on the project, I learned how to work with trees, mastered the method of recursive descent for syntactic processing of expressions.
This program builds a tree based on a mathematical expression written in a text file. The tree can be represented graphically using graphviz.
A function is implemented that decomposes expression to a given precision.
## Build and Run
1) To represent the expression tree graphically:
```
sudo apt-get install graphviz
```
2) Cloning repository:
```
git clone git@github.com:maksimra/differentiator.git
```
3) Go to the new folder:
```
cd differentiator
```
4) To compile, type:
```
make
```
5) To run the project:
```
objects/output "name_of_file_with_expression.txt"
```
## Usage
You should create .txt file which contains math expression (in folder with repository).
***At the end of the expression should be a sign '$'!***
### Example:
![image](https://github.com/user-attachments/assets/2cd59ff9-66e1-4bc2-a32a-5387b6785caa)
