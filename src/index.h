R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Taillight control</title>
    <link href="https://www.jqueryscript.net/css/jquerysctipttop.css" rel="stylesheet" type="text/css">
    <link rel="icon" href="data:,">
    <script src="https://code.jquery.com/jquery-2.2.0.min.js"></script>
</head>
<body>
<div class="main">
    <div class="slidecontainer">
        <table>
            <tr>
                <th>Name</th>
                <th>On/Off</th>
                <th>Value</th>
            </tr>
            %BUTTON_REPLACE%
        </table>
    </div>

    <form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>
    <input type='file' name='update' accept=".bin" id='file'>
    <input type='submit' class=btn value='Update'>
    <br>
        <div id='prgbar'>
            <div id='bar'></div>
        </div>
    <br>
    <p>Compiled on %DATE%</p>
</form>
</div>
<script type="text/javascript">
    // Light control block (update)
    $("input" ).on( "change", function( event ) {
        var value = null
        var name = event.target.id
        var url = "./update?name=" + name + "&"
        if (event.target.type === "checkbox") {
            console.log("Checkbox")  // value is "on". better take checked
            value = this.checked
            url += "state=" + Number(value)
        } else if (event.target.type === "range") {
            console.log("Slider")
            value = this.value
            url += "value=" + Number(value)
        } else {
            console.log("Unknown source called from " + event.target.id + " with value " + this.value)
            return;
        }
        console.log(event.target.type + " called from " + name + " with value " + value)
        const Http = new XMLHttpRequest();
        console.log(url);
        Http.open("GET", url);
        Http.send();
        Http.onreadystatechange=(e)=>{
            console.log(Http.responseText)
        }
    });

    // Updater block (upload)
    $('form').submit(function(e){
        console.log("Starting update");
        e.preventDefault();
        var form = $('#upload_form')[0];
        var data = new FormData(form);
        $.ajax({
        url: '/upload',
        type: 'POST',
        data: data,
        contentType: false,
        processData:false,
        xhr: function() {
            var xhr = new window.XMLHttpRequest();
            xhr.upload.addEventListener('progress', function(evt) {
                if (evt.lengthComputable) {
                    var per = evt.loaded / evt.total;
                    console.log(per);
                    $('#prg').html('progress: ' + Math.round(per*100) + '%');
                    $('#bar').css('width',Math.round(per*100) + '%');
                }
            }, false);
            return xhr;
        },
        success:function(d, s) {
            console.log('success!'
            location.reload();    
        },
            error: function (a, b, c) {
            console.log('error!')    
            location.reload();    
        }
        });
    });
</script>
</body>
</html>
)rawliteral";
