#include <co.hpp>
#include <cstdio>
#include <memory>
#include <string>

using namespace std;

int f1();
string f2();

int f1() {
  // main call next
  int n;
  string s;
  co::AnyPtr p;

  printf("f1 start\n");

  p = co::yield(make_shared<string>("1"));
  // main call next
  assert(!p);

  auto c = co::async(f2);

  p = co::await(c);
  // main call next
  assert(!p);

  p = co::yield(make_shared<int>(4));
  // main call next
  assert(!p);

  printf("f1 return\n");
  return 1;
}

string f2() {
  // f1 call yield from
  int n;
  string s;
  co::AnyPtr p;
  printf("f2 start\n");

  p = co::yield(make_shared<int>(2));
  // main send 1
  n = *static_pointer_cast<int>(p);
  assert(n == 1);

  p = co::yield(make_shared<string>("3"));
  // main send "2"
  s = *static_pointer_cast<string>(p);
  n = stoi(s);
  assert(n == 2);

  printf("f2 return\n");
  return "2";
}

int main() {
  int n;
  string s;
  co::AnyPtr p;
  auto c = co::async(f1);

  p = c->next();
  // f1 yield "1"
  s = *static_pointer_cast<string>(p);
  n = stoi(s);
  assert(n == 1);

  p = c->next();
  // f2 yield 2
  n = *static_pointer_cast<int>(p);
  assert(n == 2);

  p = c->send(make_shared<int>(1));
  // f2 yield 3
  s = *static_pointer_cast<string>(p);
  n = stoi(s);
  assert(n == 3);

  p = c->send(make_shared<string>("2"));
  // f2 return "2"
  s = *static_pointer_cast<string>(p);
  n = stoi(s);
  assert(n == 2);

  p = c->next();
  // f1 yield 4
  n = *static_pointer_cast<int>(p);
  assert(n == 4);

  p = c->next();
  // f1 return 1
  n = *static_pointer_cast<int>(p);
  assert(n == 1);
}
