#if !defined(__DB_DYNAMIC_H__)
#define __DB_DYNAMIC_H__

class StatsDbWrite : public D_Object
{
private:
    StatsDb *m_Db;

public:
    StatsDbWrite(StatsDb *db) : D_Object(D_VARIABLE)
    {
        m_Db = db;
        writeable = TRUE;
    }

    virtual void WriteAsString(const std::string& in)
    {
#if defined(TRACE_SQL)
        printf("WriteAsString writes \"%s\"\n", in.c_str());
#endif /* TRACE_SQL */
    
        // JDL: Sample code for performing a write in parallel.  
        // Note that no buffering, syncing, etc is used.  This 
        // might not be necessary any longer.

        m_Db->driver->exec(in);
   } 
} ;

class StatsDbRead : public D_Object
{
private:
    StatsDb *m_Db;

public:
    StatsDbRead(StatsDb *db) : D_Object(D_COMMAND)
    {
        m_Db = db;
        executable = TRUE;
    }

    virtual void ExecuteAsString(const std::string& in, 
                                 std::string& out) 
    {
#if defined(TRACE_SQL)
        printf("ExecuteAsString receives query: \"%s\"\n", in.c_str());
#endif /* TRACE_SQL */
    
        out = "";

        m_Db->driver->exec(in, out);

#if defined(DEBUG_MARSHALL)
        printf("ExecuteAsString returns response \"%s\"\n",
               out.c_str());
#endif /* DEBUG_MARSHALL */
    }
} ;

#endif
