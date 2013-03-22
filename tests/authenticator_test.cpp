
#include <iostream>
#include <stdio.h>
#include <authenticator.hpp>

using namespace std;

int main()
{
    Authenticator a(4);

    cout << "Send nonce to client: " << a.nonce() << endl;
    if (a.authenticate("dario", "cc85dcfd134310822925c7fe52b7a310a3f77247"))
        cout << "AUTH OK\n";
    else
        cout << "AUTH FAIL\n";
    return 0;
}
