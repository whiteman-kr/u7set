StaticLibInitializer
{
    void initialize()
    {
        static Q_INIT_RESOURCE(preview);
    }

    StaticLibInitializer()
    {
         initialize();
    }
}

