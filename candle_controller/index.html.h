const char INDEX_HTML_TEMPLATE[] PROGMEM = R"INDEX_HTML_EOF(
<!DOCTYPE html>
<html lang="en">
    <head>
        <title>Test Page</title>
        <link rel="stylesheet" type="text/css" href="spectrum.css">
        <style type="text/css">
            #candle-button-list > button
            {
                margin-right: 5px;
            }

            button.selected
            {
                background-color: green;
            }
        </style>
        <script type="text/javascript" src="jquery.js"></script>
        <script type="text/javascript" src="spectrum.js"></script>
        <script type="text/javascript">
            $(() => {
                const numCandles = %d;
                const thisIPAddress = window.location.host;
                let selectedTarget = null;

                for(let i = 0; i < numCandles; ++i)
                {
                    let btnNum = i, candleLetter = String.fromCharCode('A'.codePointAt(0) + i);
                    $('#candle-button-list').append($('<button type="click"></button>').text(candleLetter).click(function() {
                        let addrParts = thisIPAddress.split('.');
                        addrParts[3] = (201 + btnNum).toString();
                        let targetIP = addrParts.join('.');
                        let thisBtn = $(this);
                        selectedTarget = null;
                        $('#palette-submit-button, #reset-button').attr('disabled', 'disabled');
                        $('#candle-button-list').children().removeClass('selected');
                        $.post(`http://${targetIP}/blink`).done(() => {
                            selectedTarget = targetIP;
                            thisBtn.addClass('selected');
                            $('#palette-submit-button, #reset-button').removeAttr('disabled');
                        }).fail(() => alert(`Connection to candle ${candleLetter} at ${addrParts.join('.')} failed.`));
                        // alert('Clicked button ' + btnNum);
                    }));
                }

                $('#palette-color1, #palette-color2, #palette-color3').spectrum({
                    flat: true,
                    showPalette: true,
                    showInput: true
                });

                $('#reset-button').on('click', () => $.post(`http://${selectedTarget}/set_color_palette`, {colors: []})
                                                          .fail(() => alert('Failed to reset color palette.')))

                $('#set-palette-form').on('submit', event => {
                    event.preventDefault();
                    const currentColors = [$('#palette-color1'), $('#palette-color2'), $('#palette-color3')].map(picker => {
                        const colorObj = picker.spectrum('get').toHsv();
                        return `${Math.round((colorObj.h / 360.0) * 255.0)},${Math.round(colorObj.s * 255.0)},${Math.round(colorObj.v * 255.0)}`;
                    }).join(';');
                    $.post(`http://${selectedTarget}/set_color_palette`, {colors: currentColors})
                        .fail(() => alert('Failed to set color palette.'));
                });

                // for(let button of document.getElementById('candle-button-list').children)
                // {
                //     new xmlHttpRequest({});
                // }
            });
        </script>
    </head>
    <body>
        <p>Select a candle to control:</p>
        <div id="candle-button-list"></div>
        <p>Select palette colors:</p>
        <button type="button" id="reset-button" disabled="disabled">Reset</button><br />
        <form id="set-palette-form">
            <input type="text" id="palette-color1" value="red" />
            <input type="text" id="palette-color2" value="orange" />
            <input type="text" id="palette-color3" value="yellow" />
            <button type="submit" id="palette-submit-button" disabled="disabled">Set palette</button>
        </form>
    </body>
</html>
)INDEX_HTML_EOF";

void index_html_render(WebServer& server, size_t numCandles)
{
    char* responseBuf = new char[sizeof(INDEX_HTML_TEMPLATE) + 20];
    sprintf(responseBuf, INDEX_HTML_TEMPLATE, numCandles);
    server.send(200, "text/html", responseBuf);
    delete[] responseBuf;
}
