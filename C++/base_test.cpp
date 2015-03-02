#include <string>
#include <iostream>

using namespace std;

class SchoolMember
{
public:
	int age;
	string name;
	SchoolMember(string, int);
//protected:
	virtual void tell();
};

SchoolMember::SchoolMember(string name, int age)
	:name(name), age(age)
{
}

void SchoolMember::tell(){
	cout << "Name: " << name << "Age: " << age << endl;
}

class Teacher: public SchoolMember
{
public:
	float salary;
	Teacher(string, int, float);
	void tell();
};

Teacher::Teacher(string name, int age, float salary)
	:SchoolMember(name, age), salary(salary)
{
}

void Teacher::tell(){
	cout << "Name: " << name << "Age: " << age << "Salary: " << salary << endl;
}

class Student: public SchoolMember
{
public:
	int marks;
	Student(string, int, int);
	void tell();
};

Student::Student(string name, int age, int marks)
	:SchoolMember(name, age), marks(marks)
{
}

void Student::tell(){
	cout << "Name: " << name << "Age: " << age << "Marks: " << marks << endl;
}

int main(){
	Student *stu = new Student("lilei", 24, 100);
	stu->tell();

	Teacher *tch = new Teacher("fengzhong", 35, 10000.0);
	tch->tell();

	SchoolMember *tmp = stu;
	tmp->tell();

	tmp = tch;
	tmp->tell();

	return 0;
}

