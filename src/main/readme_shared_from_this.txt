
// enable_shared_from_this

// The problem

// because the built-in `this` pointer itself is not with RAII ability.
// the pointer `this` is same as the object address taken by &obj pointer.

struct T{
  auto getthis(){
    return this;
  }
};

int main() {
  T t;
  std::cout << &t << '\n';
  std::cout << t.getthis() << '\n';
  return 0;
}


---


// https://learn.microsoft.com/en-us/cpp/standard-library/enable-shared-from-this-class?view=msvc-170

#include <memory>
#include <iostream>
#include <cstring>

using namespace std;

struct base
  // : public std::enable_shared_from_this<base>
{
  char *val;
  base(){
    val = new char[1024];
  }
  ~base(){
    delete []val;
  }
  // shared_ptr<base> share_more()
  base *share_more()
  {
    // return shared_from_this();
    return this;
  }
};

int main1()
{
  // std::shared_ptr<base> sp2;
  base *sp2;
  {
    // std::shared_ptr<base> sp1 = make_shared<base>();
    base b; base *sp1 = &b;
    sp2 = sp1->share_more();

    std::strncpy(sp1->val, "hello", 1024 - 1);
  }
  cout << "sp2->val == " << sp2->val << endl; // Line:38
  return 0;
}

/*
==13905==ERROR: AddressSanitizer: heap-use-after-free on address ...
  #2 0x558a4a93820f in main /home/ljh/Documents/hello_cpp/src/main2.cpp:38
*/


---


// The solution

// https://learn.microsoft.com/en-us/cpp/standard-library/enable-shared-from-this-class?view=msvc-170

#include <memory>
#include <iostream>
#include <cstring>

using namespace std;

struct base
  : public std::enable_shared_from_this<base>
{
  char *val;
  base(){
    val = new char[1024];
  }
  ~base(){
    delete []val;
  }
  shared_ptr<base> share_more()
  // base *share_more()
  {
    return shared_from_this();
    // return this;
  }
};

int main2()
{
  std::shared_ptr<base> sp2;
  // base *sp2;
  {
    std::shared_ptr<base> sp1 = make_shared<base>();
    // base b; base *sp1 = &b;
    sp2 = sp1->share_more();

    std::strncpy(sp1->val, "hello", 1024 - 1);
  }
  cout << "sp2->val == " << sp2->val << endl;
  return 0;
}


---


chatGPT, 7/12/2023, 12:56:52 AM

Yes, the enable_shared_from_this class template in C++ is used to solve
the problem where the built-in this pointer cannot be used with
RAII (Resource Acquisition Is Initialization) classes.

In C++, when using smart pointers such as shared_ptr, an object can be
owned by multiple shared pointers. In some cases, you may want to obtain
a shared_ptr to this inside a member function of a class that is managed
by shared_ptr. However, using shared_ptr in the constructor or destructor
of the class can lead to issues because the object may not be fully
constructed or may have been partially destructed when those functions are called.

The enable_shared_from_this class provides a solution to this problem.
By inheriting from enable_shared_from_this in your class, you can use
the shared_from_this member function to obtain a shared_ptr to the
owning object instead of relying on the this pointer.

Here is an example:

#include <memory>

class MyClass : public std::enable_shared_from_this<MyClass> {
public:
    std::shared_ptr<MyClass> getShared() {
        return shared_from_this();
    }
};

int main() {
    std::shared_ptr<MyClass> obj = std::make_shared<MyClass>();
    std::shared_ptr<MyClass> sharedObj = obj->getShared();

    // Now both 'obj' and 'sharedObj' share ownership of the same object

    return 0;
}

In the above example, enable_shared_from_this allows MyClass objects to
obtain a shared_ptr to themselves without the risk of premature destruction.
This ensures that the object remains valid as long as any shared_ptr points to it,
providing RAII semantics.


---

