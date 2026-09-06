 #include <iostream>
 #include <string>

 class TestClass {
 public:
     TestClass(const std::string& name);
     void greet() const;
     void setValue(int val);
     int getValue() const;
 private:
     std::string m_name;
     int m_value;
 };

 TestClass::TestClass(const std::string& name) : m_name(name), m_value(0) {
     std::cout << "TestClass object " << m_name << " created." << std::endl;
 }

 void TestClass::greet() const {
     std::cout << "Hello from " << m_name << "! Value = " << m_value << std::endl;
 }

 void TestClass::setValue(int val) {
     m_value = val;
 }

 int TestClass::getValue() const {
     return m_value;
 }

 int main() {
     TestClass obj("MyObject");
     obj.greet();
     obj.setValue(42);
     obj.greet();
     return 0;
 }