using NUnit.Framework;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Linq;
using ZBarImg.Test.Utils;

namespace ZBarImg.Test
{
    public class SomeDecodeTests
    {
        [Test, TestCaseSource(nameof(GetSamplesCode39ExtendedN))]
        public void DecodeSomeCode39ExtendedN(string pngFile)
        {
            var bytes = zbarimg(pngFile);
            Assert.AreEqual(
                Path.GetFileNameWithoutExtension(pngFile),
                BinUtils.FullyUrlEncode(bytes)
            );
        }

        [Test, TestCaseSource(nameof(GetSamplesCode39ExtendedY))]
        public void DecodeSomeCode39ExtendedY(string pngFile)
        {
            var bytes = zbarimg(pngFile, "-Scode39.ascii=1");
            Assert.AreEqual(
                Path.GetFileNameWithoutExtension(pngFile),
                BinUtils.FullyUrlEncode(bytes)
            );
        }

        private byte[] zbarimg(string pngFile, string opts = null)
        {
            var thisDLL = new Uri(GetType().Assembly.CodeBase).LocalPath;
            var configuration = Path.GetFileName(Path.GetDirectoryName(thisDLL));
            var zbarimgExe = Path.GetFullPath(Path.Combine(thisDLL, "..", "..", "..", "..", configuration, "zbarimg.exe"));

            var psi = new ProcessStartInfo(zbarimgExe, $" --xml {opts} \"{pngFile}\"")
            {
                UseShellExecute = false,
                RedirectStandardOutput = true,
                StandardOutputEncoding = Encoding.GetEncoding("latin1"),
            };
            var p = Process.Start(psi);
            var xmlBody = p.StandardOutput.ReadToEnd();
            p.WaitForExit();
            var xml = XDocument.Load(new StringReader(xmlBody));
            var ns = XNamespace.Get("http://zbar.sourceforge.net/2008/barcode");
            foreach (var data in xml.Document
                .Elements(ns + "barcodes")
                .Elements(ns + "source")
                .Elements(ns + "index")
                .Elements(ns + "symbol")
                .Elements(ns + "data")
                )
            {
                if (data.Attribute("format")?.Value == "base64")
                {
                    return Convert.FromBase64String(data.Value);
                }
                else
                {
                    return Encoding.GetEncoding("latin1").GetBytes(data.Value);
                }
            }

            return new byte[0];
        }

        public static IEnumerable<string> GetSamplesCode39ExtendedN() =>
            Directory.GetFiles("Samples/CODE-39/ExtendedN")
                .Select(it => Path.GetFullPath(it));

        public static IEnumerable<string> GetSamplesCode39ExtendedY() =>
            Directory.GetFiles("Samples/CODE-39/ExtendedY")
                .Select(it => Path.GetFullPath(it));
    }
}
