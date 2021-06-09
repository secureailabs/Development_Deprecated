using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Management.Automation;
using System.IO;
using System.Net;
using System.Text.Json;
using System.Collections.ObjectModel;
using System.Text;

namespace CleanRoomServer
{
    class Program
    {
        static void Main(string[] args)
        {
            RestServer oHttpServer = new RestServer();
            oHttpServer.Start();
        }
    }
}

class RestServer
{
    public static HttpListener listener;
    public static string url = "https://+:8000/";
    //public static string url = "https://127.0.0.1:8000/";
    Dictionary<string, string> executingPowershells = new Dictionary<string, string>();

    public class EncryptInputParams
    {
        public string AppId;
        public string Secret;
        public string TenantId;
        public string KeyURL;
        public string SubscriptionId;
        public string TransactionId;
        public string ResourceGroup;
        public string Region;
        public string SASToken;
        public string RSAKey;
        public string VMSize;
    }

    public class GetStatusInputParams
    {
        public string TransactionId;
    }

    void output_DataAdded(object sender, DataAddedEventArgs e)
    {
        Console.WriteLine("output_DataAdded: ");
        PSDataCollection<PSObject> col = (PSDataCollection<PSObject>)sender;

        Collection<PSObject> rsl = col.ReadAll();
        foreach (PSObject r in rsl)
        {
            string response;
            executingPowershells.TryGetValue(e.PowerShellInstanceId.ToString("D"), out response);
            if (null != r)
            {
                Console.WriteLine("$: "+r);
                response += r.ToString() + "\n";
            }
            executingPowershells[e.PowerShellInstanceId.ToString("D")] = response;
        }
    }

    void error_DataAdded(object sender, DataAddedEventArgs e)
    {
        Console.WriteLine("error_DataAdded: ");
        PSDataCollection<ErrorRecord> col = (PSDataCollection<ErrorRecord>)sender;

        Collection<ErrorRecord> rsl = col.ReadAll();
        foreach (ErrorRecord r in rsl)
        {
            string response;
            executingPowershells.TryGetValue(e.PowerShellInstanceId.ToString("D"), out response);
            if (null != r)
            {
                Console.WriteLine("#: " + r);
                response += r.ToString() + "\n";
            }
            executingPowershells[e.PowerShellInstanceId.ToString("D")] = response;
        }
    }

    void warning_DataAdded(object sender, DataAddedEventArgs e)
    {
        Console.WriteLine("warning_DataAdded: ");
        PSDataCollection<WarningRecord> col = (PSDataCollection<WarningRecord>)sender;

        Collection<WarningRecord> rsl = col.ReadAll();
        foreach (WarningRecord r in rsl)
        {
            string response;
            executingPowershells.TryGetValue(e.PowerShellInstanceId.ToString("D"), out response);
            if (null != r)
            {
                Console.WriteLine("@: " + r);
                response += r.ToString() + "\n";
            }
            executingPowershells[e.PowerShellInstanceId.ToString("D")] = response;
        }
    }

    void debug_DataAdded(object sender, DataAddedEventArgs e)
    {
        Console.WriteLine("debug_DataAdded: ");
        PSDataCollection<DebugRecord> col = (PSDataCollection<DebugRecord>)sender;

        Collection<DebugRecord> rsl = col.ReadAll();
        foreach (DebugRecord r in rsl)
        {
            string response;
            executingPowershells.TryGetValue(e.PowerShellInstanceId.ToString("D"), out response);
            if (null != r)
            {
                Console.WriteLine("==: " + r);
                response += r.ToString() + "\n";
            }
            executingPowershells[e.PowerShellInstanceId.ToString("D")] = response;
        }
    }

    void verbose_DataAdded(object sender, DataAddedEventArgs e)
    {
        Console.WriteLine("verbose_DataAdded: ");
        PSDataCollection<VerboseRecord> col = (PSDataCollection<VerboseRecord>)sender;

        Collection<VerboseRecord> rsl = col.ReadAll();
        foreach (VerboseRecord r in rsl)
        {
            string response;
            executingPowershells.TryGetValue(e.PowerShellInstanceId.ToString("D"), out response);
            if (null != r)
            {
                Console.WriteLine("%%: " + r);
                response += r.ToString() + "\n";
            }
            executingPowershells[e.PowerShellInstanceId.ToString("D")] = response;
        }
    }

    void progress_DataAdded(object sender, DataAddedEventArgs e)
    {
        Console.WriteLine("progress_DataAdded: ");
        PSDataCollection<ProgressRecord> col = (PSDataCollection<ProgressRecord>)sender;

        Collection<ProgressRecord> rsl = col.ReadAll();
        foreach (ProgressRecord r in rsl)
        {
            string response;
            executingPowershells.TryGetValue(e.PowerShellInstanceId.ToString("D"), out response);
            if (null != r)
            {
                Console.WriteLine("%%: " + r);
                response += r.ToString() + "\n";
            }
            executingPowershells[e.PowerShellInstanceId.ToString("D")] = response;
        }
    }

    void information_DataAdded(object sender, DataAddedEventArgs e)
    {
        Console.WriteLine("progress_DataAdded: ");
        PSDataCollection<InformationRecord> col = (PSDataCollection<InformationRecord>)sender;

        Collection<InformationRecord> rsl = col.ReadAll();
        foreach (InformationRecord r in rsl)
        {
            string response;
            executingPowershells.TryGetValue(e.PowerShellInstanceId.ToString("D"), out response);
            if (null != r)
            {
                Console.WriteLine("!!: " + r);
                response += r.ToString() + "\n";
            }
            executingPowershells[e.PowerShellInstanceId.ToString("D")] = response;
        }
    }

    private async Task EncryptVirtualMachine(HttpListenerContext ctx, string jsonString)
    {
        // Deserialize Json Object
        // Convert to Json
        var options = new JsonSerializerOptions
        {
            IncludeFields = true,
        };
        var inputParameters = JsonSerializer.Deserialize<EncryptInputParams>(jsonString, options);

        // Using the PowerShell.Create and AddCommand methods, create a command pipeline.
        PowerShell ps = PowerShell.Create().AddScript("Set-ExecutionPolicy -ExecutionPolicy Unrestricted -Scope Process");
        ps.AddScript(".\\BuildVmImage.ps1 -AppId '"+ inputParameters.AppId + "' -Secret '"+ inputParameters.Secret + "' -TenantId '"+ inputParameters.TenantId +"' -KeyURL '" + inputParameters.KeyURL + "' -SubscriptionId '"+ inputParameters.SubscriptionId +"' -TransactionId '"+ ps.InstanceId.ToString("D") + "' -Region '"+ inputParameters.Region +"' -RSAKey '"+ inputParameters.RSAKey + "' -ResourceGroup '"+ inputParameters.ResourceGroup +"' -SASToken '"+ inputParameters.SASToken +"' -VmSize '"+ inputParameters.VMSize +"'", true);

        // Create a Guid for the transaction 
        executingPowershells.Add(ps.InstanceId.ToString("D"), "");

        ps.Streams.Error.DataAdded += error_DataAdded;
        ps.Streams.Debug.DataAdded += debug_DataAdded;
        ps.Streams.Warning.DataAdded += warning_DataAdded;
        ps.Streams.Progress.DataAdded += progress_DataAdded;
        ps.Streams.Verbose.DataAdded += verbose_DataAdded;
        ps.Streams.Information.DataAdded += information_DataAdded;

        // Using the PowerShell.Invoke method, run the command pipeline using the supplied input.
        string responseCode = "404";
        PSDataCollection<PSObject> output = new PSDataCollection<PSObject>();
        output.DataAdded += output_DataAdded;

        ps.BeginInvoke<PSObject, PSObject>(null, output);

        Collection<ErrorRecord> error = ps.Streams.Error.ReadAll();
        foreach (ErrorRecord r in error)
        {
            if (null != r)
            {
                Console.WriteLine("Error: ", r.ToString());
            }
        }

        // Write the response info
        HttpListenerResponse resp = ctx.Response;
        byte[] data = Encoding.UTF8.GetBytes(String.Format("{{\"Status\":\"{0}\", \"TransactionId\": \"{1}\" }}", responseCode, ps.InstanceId.ToString("D")));
        resp.ContentType = "text/html";
        resp.ContentEncoding = Encoding.UTF8;
        resp.ContentLength64 = data.LongLength;

        // Write out to the response stream (asynchronously), then close it
        await resp.OutputStream.WriteAsync(data, 0, data.Length);
        resp.Close();
    }

    private async Task GetUpdate(HttpListenerContext ctx, string jsonString)
    {
        // Deserialize Json Object
        // Convert to Json
        var options = new JsonSerializerOptions
        {
            IncludeFields = true,
        };
        var inputParameters = JsonSerializer.Deserialize<GetStatusInputParams>(jsonString, options);
        Console.WriteLine("inputParameters " + inputParameters.TransactionId);

        // Using the PowerShell.Create and AddCommand methods, create a command pipeline.
        bool isTransactionPresent = executingPowershells.TryGetValue(inputParameters.TransactionId, out string responseString);

        string responseCode = "404";
        if (isTransactionPresent)
        {
            responseCode = "201";
        }

        // Write the response info
        HttpListenerResponse resp = ctx.Response;
        byte[] data = Encoding.UTF8.GetBytes(String.Format("{{\"Status\":\"{0}\", \"Response\": \"{1}\"}}", responseCode, responseString));
        resp.ContentType = "text/html";
        resp.ContentEncoding = Encoding.UTF8;
        resp.ContentLength64 = data.LongLength;

        // Write out to the response stream (asynchronously), then close it
        await resp.OutputStream.WriteAsync(data, 0, data.Length);
        resp.Close();
    }

    public async Task HandleIncomingConnections()
    {
        bool runServer = true;

        // While a user hasn't visited the `shutdown` url, keep on handling requests
        while (runServer)
        {
            // Will wait here until we hear from a connection
            HttpListenerContext ctx = await listener.GetContextAsync();

            // Peel out the requests and response objects
            HttpListenerRequest req = ctx.Request;

            // Print out some info about the request
            Console.WriteLine(req.Url.ToString());
            Console.WriteLine(req.HttpMethod);
            Console.WriteLine(req.UserHostName);
            Console.WriteLine(req.UserAgent);
            Console.WriteLine();

            System.IO.Stream body = req.InputStream;
            System.Text.Encoding encoding = req.ContentEncoding;
            System.IO.StreamReader reader = new System.IO.StreamReader(body, encoding);
            if (req.ContentType != null)
            {
                Console.WriteLine("Client data content type {0}", req.ContentType);
            }
            Console.WriteLine("Client data content length {0}", req.ContentLength64);

            Console.WriteLine("Start of client data:");
            // Convert the data to a string and display it on the console.
            string s = reader.ReadToEnd();
            Console.WriteLine(s);
            Console.WriteLine("End of client data:");
            body.Close();
            reader.Close();

            Task encryptTask;
            if ((req.HttpMethod == "POST") && (req.Url.AbsolutePath == "/encryptAndUpload"))
            {
                Console.WriteLine("Encryption requested");
                encryptTask = EncryptVirtualMachine(ctx, s);
            }

            if ((req.HttpMethod == "POST") && (req.Url.AbsolutePath == "/getUpdate"))
            {
                Console.WriteLine("Refresh requested");
                encryptTask = GetUpdate(ctx, s);
            }
        }
    }

    public void Start()
    {
        // Create a Http server and start listening for incoming connections
        listener = new HttpListener();
        listener.Prefixes.Add(url);
        listener.Start();
        Console.WriteLine("Listening for connections on {0}", url);

        // Handle requests
        Task listenTask = HandleIncomingConnections();
        listenTask.GetAwaiter().GetResult();

        // Close the listener
        listener.Close();
    }
}
