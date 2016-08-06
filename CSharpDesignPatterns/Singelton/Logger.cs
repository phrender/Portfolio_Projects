using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace CSharpDesignPatterns.Singelton
{
    class Logger
    {
        private static Logger m_instance = null;
        private Stream m_logStream = null;
        private StreamWriter m_streamWriter = null;
        private static object m_handle = new object();
        
        protected Logger()
        {
            m_logStream = File.Open("logfile.log", FileMode.Create);
            m_streamWriter = new StreamWriter(m_logStream);
        }

        public void Log(string strMessage)
        {
            m_streamWriter.Write(strMessage);
            m_streamWriter.Write(Environment.NewLine);
        }

        ~Logger()
        {
            try
            {
                m_streamWriter.Close();
                m_streamWriter.Dispose();
            }
            catch (Exception)
            {
            }
        }

        public static Logger Instance
        {
            get
            {
                // "Thread-safe" lazy initialization
                // !IMPORTANT! Remove this lock(), if your code is not multi-threaded !IMPORTANT!
                lock (m_handle)
                {
                    if (m_instance == null)
                    {
                        m_instance = new Logger();
                    }
                }

                return m_instance;
            }
        }
    }
}
