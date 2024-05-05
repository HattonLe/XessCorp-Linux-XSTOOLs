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

    // Attempt to access the XSTOOLs parameters,
    // return true if they are accessible.
    static bool FindParameterFile(const char *UserName);

    static void InitialisationDone();

private:
    static bool InInitialisation;

};

#endif // PARAMETERS_H
