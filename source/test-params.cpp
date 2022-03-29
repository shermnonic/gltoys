#include <iostream>
#include <fstream>
#include <utils/ParameterTypes.h>
#include <utils/ParameterIO.h>

struct Bar
{
    DoubleParameter d0{ "d0", 42.3, 0.0, 100.0, 0.0 };
    DoubleParameter d1{ "d1", 23.7, 10.0, 50.0, 0.0 };
    EnumParameter et{ "test-enum", 2, { "alt1", "alt2", "alt3", "alt4", "alt5" }, 0 };
    IntParameter index{ "index", 77, 0, 100, 0 };
    StringParameter title{ "title", "Foobar" };
    ParameterList parms{ "Test Parameters", ParameterVector{ &d0, & d1, &et, &index, & title } };

    void test()
    {
        using namespace std;

        cout << "Defined following list of parameters:" << endl;
        print_params(parms);
        cout << endl;

#ifdef PARAMETERBASE_BOOST_PTREE_SERIALIZATION
        string filename("foobar.xml");

        cout << "Writing to " << filename << "..." << endl;
        save_params(filename.c_str(), parms);

        cout << "Loading again from disk (with changed ordering)..." << endl;
        load_params(filename.c_str(), parms2);
#endif

#ifdef PARAMETERBASE_JSON_SERIALIZATION
        {
            nlohmann::json j;
            appendParameterListToJsonCollection(parms, j);
            std::ofstream of("foobar.json");
            of << j.dump(2);
            of.close();

            cout << "JSON:" << endl;
            cout << j.dump(2);
            cout << endl;
        }

        parms.reset();
        title.setValue("none");

        cout << "Reset parameters:" << endl;
        print_params(parms);
        cout << endl;

        {
            nlohmann::json j;
            std::ifstream fin("foobar.json");
            fin >> j;
            fin.close();
            readParameterListFromJsonCollection(j, parms);
        }

        cout << "Loaded parameters:" << endl;
        print_params(parms);
        cout << endl;
#endif
    }
};

int main(int argc, char* argv[])
{
    using namespace std;

    Bar bar;
    bar.test();

    return EXIT_SUCCESS;
}