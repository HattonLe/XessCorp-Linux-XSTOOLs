#include "parameters.h"

#include <string.h>

#include "xserror.h"
#include "utils.h"

bool Parameters::InInitialisation = true;

Parameters::Parameters()
{

}

// Attempt to access the XSTOOLs parameters,
// return true if they are accessible.
bool Parameters::FindParameterFile(const char *UserName)
{
    XSError err(cerr);
    char UserPath[200];
    FILE* fp;

    fp = NULL;

    // We need a user name in order for root to access the correct home directory
    if (0 != strlen(UserName))
    {
        // Build root a path to the users Documents area where the Xess parameters are held
        std::snprintf(UserPath, sizeof(UserPath), "/home/%s/Documents/XessData", UserName);

        // for testing only
        setenv("XSTOOLS", UserPath, true);
        setenv("XSTOOLS_BIN_DIR", "/opt", 1);
        setenv("XSTOOLS_DATA", UserPath, true);
        setenv("XSTOOLS_DATA_DIR", UserPath, true);

        // check the XS parameter file
        string XSTOOLSParameterFilename = (string)FindXSTOOLSBinDir() + (string)"/XSPARAM.TXT";
        fp = fopen(XSTOOLSParameterFilename.c_str(),"r");
        if (fp == NULL)
        {
            char Msg[200];

            std::snprintf(Msg, sizeof(Msg), "XSTOOLs Parameter file - Error:%d could not be opened!\n\"%s\"\n", errno, XSTOOLSParameterFilename.c_str());
            err.SimpleMsg(XSErrorMajor, Msg);
        }
        else
        {
            fclose(fp);
        }
    }
    else
    {
        char Msg[200];

        strcpy(Msg, "Users name command line argument missing!\ne.g XYZ for accessing \"/home/XYZ/Documents/XessData\"\n");
        err.SimpleMsg(XSErrorMajor, Msg);
    }
    return (NULL != fp);
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
    char Msg[200];

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
    char Msg[200];

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

        while (fgets(line, 511, fp) != NULL)
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

