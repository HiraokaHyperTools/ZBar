using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ZBarImg.Test.Utils
{
    internal class BinUtils
    {
        internal static string FullyUrlEncode(string text)
        {
            return string.Join(
                "",
                Encoding.UTF8.GetBytes(text)
                    .Select(it => "%" + it.ToString("x2"))
            );
        }

        internal static string FullyUrlEncode(byte[] bytes)
        {
            return string.Join(
                "",
                bytes
                    .Select(it => "%" + it.ToString("x2"))
            );
        }
    }
}
