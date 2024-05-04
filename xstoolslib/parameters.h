#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <string>

using namespace std;

class Parameters
{
public:
    Parameters();

    static bool SetXSTOOLSParameter(const char *RequiredKey, const char *value);
    static string GetXSTOOLSParameter(const char *RequiredKey);

    static void InitialisationDone();

private:
    static bool InInitialisation;

};

#endif // PARAMETERS_H
