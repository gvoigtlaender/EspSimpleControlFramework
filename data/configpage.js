// https://stackoverflow.com/questions/22577457/update-data-on-a-page-without-refreshing
window.addEventListener('load', function()
{
    var xhr_title = null;
    var xhr_devicename = null;
    var xhr_content = null;
    
    updateLiveData_Content = function()
    {
        var url = 'configcontent';
        xhr_status = new XMLHttpRequest();
        xhr_status.onreadystatechange = evenHandler_content;
        // asynchronous requests
        xhr_status.open("GET", url, true);
        // Send the request over the network
        xhr_status.send(null);
    };

    updateLiveData_Once = function()
    {
        xhr_title = new XMLHttpRequest();
        xhr_title.open("GET", "title", true);
        // Send the request over the network
        xhr_title.onreadystatechange = evenHandler_title;
        // asynchronous requests
        xhr_title.send(null);
    };

    updateLiveData_Once();

    function evenHandler_content()
    {
        if(xhr_status.readyState == 4 && xhr_status.status == 200)
        {
            dataDiv = document.getElementById('configuration');
            dataDiv.innerHTML = xhr_status.responseText;
            // window.setTimeout(updateLiveData_Status, 500);
        }
    }
    function evenHandler_title()
    {
        // Check response is ready or not
        if(xhr_title.readyState == 4 && xhr_title.status == 200)
        {
            document.title = xhr_title.responseText;
            // document.getElementById('title').innerHTML = xhr_title.responseText;
            document.getElementById('title2').innerHTML = xhr_title.responseText;

            window.setTimeout(updateLiveData_Content, 1);
        }
    }
});