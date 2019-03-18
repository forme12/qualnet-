
#if !defined(__UTIL_GESTALT_H__)
#define __UTIL_GESTALT_H__

#include <set>
#include <map>


namespace UTIL { namespace Gestalt {



    namespace Support
    {
        class Attribute;

        typedef std::map<std::string, Attribute> MapType;
        typedef std::set<std::string> SetType;

        class Attribute
            {
                std::string attr;
                std::string val;
                bool valid;

            public:
                Attribute() { }
                Attribute(std::string p_name)
                : attr(p_name), val(""), valid(false)
                {

                }

                Attribute(std::string p_name, std::string p_val)
                : attr(p_name), val(p_val), valid(true)
                {

                }

                Attribute(std::string p_name,
                          const NodeInput* nodeInput)
                : attr(p_name), val(""), valid(false)
                {
                    BOOL wasFound(FALSE);
                    char buf[MAX_STRING_LENGTH];

                    IO_ReadString(ANY_NODEID,
                                  ANY_ADDRESS,
                                  nodeInput,
                                  attr.c_str(),
                                  &wasFound,
                                  buf);

                    if (wasFound == TRUE)
                    {
                        val = buf;
                        valid = true;
                    }
                }

                Attribute& operator=(std::string p_val)
                {
                    val = p_val;

                    valid = true;

                    return *this;
                }

                int to_i()
                {
                    return atoi(val.c_str());
                }

                float to_f()
                {
                    return (float)atof(val.c_str());
                }

                std::string to_s()
                {
                    return val;
                }

                std::string get_attr()
                {
                    return attr;
                }

                bool isValid()
                {
                    return valid;
                }
            } ;

        extern MapType attr_map;
        extern SetType attr_set;

        static bool supports(std::string attr)
        {
            Support::SetType::iterator pos
                = Support::attr_set.find(attr);

            if (pos != Support::attr_set.end())
                return true;

            return false;
        }

        static void add(std::string attr, std::string val)
        {
            Attribute a(attr, val);

            attr_map[attr] = a;

            attr_set.insert(attr);
        }

        static void add(const NodeInput* nodeInput,
                        std::string attr,
                        std::string def)
        {
            Attribute a(attr, nodeInput);

            if (a.isValid())
            {
                attr_map[attr] = a;
            }
            else
            {
                attr_map[attr] = def;
            }

            attr_set.insert(attr);

        }

        static Attribute& get(std::string attr)
        {
            MapType::iterator pos = attr_map.find(attr);

            if (pos != attr_map.end())
            {
                return pos->second;
            }
            else
            {
                char buf[BUFSIZ];

                sprintf(buf,
                        "Cannot find value for attribute"
                        " %s.\n",
                        attr.c_str());

                ERROR_ReportError(buf);
            }

            return pos->second;
        }

        static void initialize(const NodeInput *nodeInput,
                               int numberOfProcessors,
                               char* experimentPrefix)
        {
            add(nodeInput, "GESTALT-USE-WORKER-THREAD", "FALSE");
            add(nodeInput, "GESTALT-PREFER-SHARED-MEMORY" , "FALSE");
        }

        static void finalize()
        {

        }

    }

    static bool get_b(std::string attr)
    {
        Support::Attribute& a = Support::get(attr);
        std::string str = a.to_s();

        if (str == "yes" || str == "YES"
            || str == "TRUE" || str == "true"
            || str == "Yes" || str == "True"
            || str == "on" || str == "ON" || str == "On")
        {
            return true;
        }

        if (str == "no" || str == "NO"
            || str == "FALSE" || str == "false"
            || str == "No" || str == "False"
            || str == "off" || str == "OFF" || str == "Off")
        {
            return false;
        }

        ERROR_ReportError("Unknown boolean keyword");
        return false;
    }

    static int get_i(std::string attr)
    {
        Support::Attribute& a = Support::get(attr);

        return a.to_i();
    }

    static float get_f(std::string attr)
    {
        Support::Attribute& a = Support::get(attr);

        return a.to_f();
    }

    static std::string get_s(std::string attr)
    {
        Support::Attribute& a = Support::get(attr);

        return a.to_s();
    }

} }

#endif /* __UTIL_GESTALT_H__ */

