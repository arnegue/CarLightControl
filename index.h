R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Headlight control</title>
    <link href="https://www.jqueryscript.net/css/jquerysctipttop.css" rel="stylesheet" type="text/css">
	<link rel="icon" href="data:,">
    <script src="https://code.jquery.com/jquery-2.2.0.min.js"></script>
</head>
<body>
<!-- Amino slider elements -->
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
</div>

<!-- dependencies -->
<script type="text/javascript">
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
        //const url='./update?name=' + name + "&value;
        console.log(url);
        Http.open("GET", url);
        Http.send();
        Http.onreadystatechange=(e)=>{
            console.log(Http.responseText)
        }
    });
</script>
</body>
</html>
)rawliteral";