#include "parameters.h"

#include <string.h>

#include "xserror.h"
#include "utils.h"

bool Parameters::InInitialisation = true;

Parameters::Parameters()
{

}

void Parameters::InitialisationDone()
{
    XSError err(cerr);
    char Msg[100];

    InInitialisation = false;
    std::snprintf(Msg, sizeof(Msg), "Parameters Initialisation Done\n");
    err.SimpleMsg(XSErrorNone, Msg);
}

/// Set XSTOOLs parameter value.
///\return true if successful; false otherwise
bool Parameters::SetXSTOOLSParameter(const char *RequiredKey, const char *value)
{
    XSError err(cerr);
    FILE* fp;
    char Msg[100];

    fp = NULL;

    // Ignore any changes requested during GUI initialisation time
    if (InInitialisation)
    {
        std::snprintf(Msg, sizeof(Msg), "SetXSTOOLSParameter Attempted Setting '%s' to '%s' during Init\n", RequiredKey, value);
        err.SimpleMsg(XSErrorDebug, Msg);
    }
    else
    {
        int numLines;
        char* lines[1000];
        string NewKey;
        int i;

        NewKey = ConvertToUpperCase((string) RequiredKey);

        if (0 == strlen(value))
        {
            std::snprintf(Msg, sizeof(Msg), "SetXSTOOLSParameter Setting empty value for '%s'\n", RequiredKey);
            err.SimpleMsg(XSErrorDebug, Msg);
        }

        // remove the parameter value from the XSPARAM file and replace it at the end of the file with a new value
        string XSTOOLSParameterFilename = (string)FindXSTOOLSBinDir() + (string)"/XSPARAM.TXT";

        numLines = 0;
        fp = fopen(XSTOOLSParameterFilename.c_str(),"r");
        if (fp != NULL)
        {
            char line[512], val[512];

            for (i = 0; fgets(line, 511, fp) != NULL; i++)
            {
                char Key[512];
                int n;

                // clear the value string
                val[0] = 0;
                n = sscanf(line, "%s %s", Key, val);
                if (n == 0 || n == EOF)
                {
                    continue;	// skip blank lines
                }
                else
                {
                    if (n != 2)
                    {
                        std::snprintf(Msg, sizeof(Msg), "SetXSTOOLSParameter corrupted record '%s'\n", line);
                        err.SimpleMsg(XSErrorMajor, Msg);

                        /* fclose(fp);

                        fp = NULL;
                        break;*/
                        lines[numLines++] = strcat(strdup(line), " CORRUPTED");
                    }
                    else
                    {
                         // retain lines that don't match the one to be replaced
                        string Entry = ConvertToUpperCase(Key);
                        if (0 != Entry.compare(NewKey))
                        {
                            lines[numLines++] = strdup(line);
                        }
                    }
                }
            }
            fclose(fp);
        }
        fp = fopen(XSTOOLSParameterFilename.c_str(), "w");
        if (fp == NULL)
        {
            std::snprintf(Msg, sizeof(Msg), "SetXSTOOLSParameter file (%s) Error:%d could not be opened!!\n", XSTOOLSParameterFilename.c_str(), errno);
            err.SimpleMsg(XSErrorMajor, Msg);
        }
        else
        {
            for (i = 0; i < numLines; i++)
            {
                fprintf(fp, "%s", lines[i]);
            }
            fprintf(fp,"%s %s\n", NewKey.c_str(), ConvertToUpperCase(value).c_str());
            fclose(fp);
        }
        for (i = 0; i < numLines; i++)
        {
           free(lines[i]);
        }

        std::snprintf(Msg, sizeof(Msg), "SetXSTOOLSParameter '%s' Set:'%s'\n", RequiredKey, value);
        err.SimpleMsg(XSErrorDebug, Msg);
    }
    return (fp != NULL);
}


/// Get XSTOOLs parameter value from registry or file in XSTOOLs directory.
///\return value of the parameter
string Parameters::GetXSTOOLSParameter(const char *RequiredKey) ///< name of parameter whose value is returned
{
    XSError err(cerr);
    string NeededKey;
    string ValueFound;
    FILE* fp;
    char Msg[80];

    NeededKey = ConvertToUpperCase((string) RequiredKey);
    ValueFound = "";

    // check the XS parameter file
    string XSTOOLSParameterFilename = (string)FindXSTOOLSBinDir() + (string)"/XSPARAM.TXT";
    fp = fopen(XSTOOLSParameterFilename.c_str(),"r");
    if (fp == NULL)
    {
        std::snprintf(Msg, sizeof(Msg), "GetXSTOOLSParameter file (%s) Error:%d could not be opened!!\n", XSTOOLSParameterFilename.c_str(), errno);
        err.SimpleMsg(XSErrorMajor, Msg);
    }
    else
    {
        char line[512], Val[512];

        while(fgets(line, 511, fp) != NULL)
        {
            // got a line of text.  now see if it starts with the name we are searching for...
            char Key[512];
            int n;

            // clear the value string
            Val[0] = 0;
            n = sscanf(line, "%s %s", Key, Val);
            if (n == 0 || n == EOF)
            {
                continue;	// skip blank lines
            }
            else
            {
                if (n != 2)
                {
                    std::snprintf(Msg, sizeof(Msg), "GetXSTOOLSParameter corrupted record '%s'\n", line);
                    err.SimpleMsg(XSErrorMajor, Msg);

                    //break;
                }
                else
                {
                    string Entry = ConvertToUpperCase(Key);
                    if (0 == Entry.compare(NeededKey))
                    {
                        ValueFound = ConvertToUpperCase(Val);
                        break;	// found the parameter name.  now return the parameter value...
                    }
                }
            }
            // otherwise keep searching through the parameter text file...
        }
        fclose(fp);
    }

    std::snprintf(Msg, sizeof(Msg), "GetXSTOOLSParameter '%s' Found:'%s'\n", RequiredKey, ValueFound.c_str());
    err.SimpleMsg(XSErrorDebug, Msg);
    return ValueFound;
}

