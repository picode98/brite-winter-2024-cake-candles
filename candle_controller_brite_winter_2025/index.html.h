const char INDEX_HTML_TEMPLATE[] PROGMEM = R"INDEX_HTML_EOF(
<!DOCTYPE html>
<html lang="en">
    <head>
        <title>Candle Controller</title>
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
                    showButtons: false,
                    showPalette: true,
                    showInput: true
                });

                $('#reset-button').on('click', () => $.post(`http://${selectedTarget}/set_color_palette`, {colors: []})
                                                          .fail(() => alert('Failed to reset color palette.')));
                $('#reset-all-button').on('click', () => {
                    let promises = [];
                    for(let i = 0; i < numCandles; ++i)
                    {
                        let addrParts = thisIPAddress.split('.');
                        addrParts[3] = (201 + i).toString();
                        let targetIP = addrParts.join('.');
                        promises.push($.post(`http://${targetIP}/set_color_palette`, {colors: []}));
                    }
                    Promise.allSettled(promises).then(results => {
                        let failed = [];
                        for(let i = 0; i < results.length; ++i)
                        {
                            if(results[i].status !== 'fulfilled')
                            {
                                failed.push(String.fromCharCode('A'.codePointAt(0) + i));
                            }
                        }

                        if(failed.length > 0)
                        {
                            alert('Resetting the following candles failed: ' + failed.join(', '));
                        }
                    });
                });

                $('#set-time-button').on('click', () => {
                    let promises = [];
                    for(let i = 0; i < numCandles; ++i)
                    {
                        let addrParts = thisIPAddress.split('.');
                        addrParts[3] = (201 + i).toString();
                        let targetIP = addrParts.join('.');
                        let now = new Date();
                        promises.push($.post(`http://${targetIP}/set_time`, {timestamp: Math.round(now.getTime() / 1000), timezone: now.getTimezoneOffset()}));
                    }
                    Promise.allSettled(promises).then(results => {
                        let failed = [];
                        for(let i = 0; i < results.length; ++i)
                        {
                            if(results[i].status !== 'fulfilled')
                            {
                                failed.push(String.fromCharCode('A'.codePointAt(0) + i));
                            }
                        }

                        if(failed.length > 0)
                        {
                            alert('Setting the time for the following candles failed: ' + failed.join(', '));
                        }
                    });
                });

                let setForAll = false;
                $('#palette-submit-button').on('click', () => { setForAll = false; });
                $('#palette-submit-all-button').on('click', () => { setForAll = true; });
                

                $('#set-palette-form').on('submit', event => {
                    event.preventDefault();
                    const currentColors = [$('#palette-color1'), $('#palette-color2'), $('#palette-color3')].map(picker => {
                        const colorObj = picker.spectrum('get').toHsv();
                        return `${Math.round((colorObj.h / 360.0) * 255.0)},${Math.round(colorObj.s * 255.0)},${Math.round(colorObj.v * 255.0)}`;
                    }).join(';');
                    if(setForAll)
                    {
                        let promises = [];
                        for(let i = 0; i < numCandles; ++i)
                        {
                            let addrParts = thisIPAddress.split('.');
                            addrParts[3] = (201 + i).toString();
                            let targetIP = addrParts.join('.');
                            promises.push($.post(`http://${targetIP}/set_color_palette`, {colors: currentColors}));
                        }
                        Promise.allSettled(promises).then(results => {
                            let failed = [];
                            for(let i = 0; i < results.length; ++i)
                            {
                                if(results[i].status !== 'fulfilled')
                                {
                                    failed.push(String.fromCharCode('A'.codePointAt(0) + i));
                                }
                            }
                            if(failed.length > 0)
                            {
                                alert('Setting color palettes for the following candles failed: ' + failed.join(', '));
                            }
                        });
                    }
                    else
                    {
                        $.post(`http://${selectedTarget}/set_color_palette`, {colors: currentColors})
                            .fail(() => alert('Failed to set color palette.'));
                    }
                });

                // for(let button of document.getElementById('candle-button-list').children)
                // {
                //     new xmlHttpRequest({});
                // }
            });
        </script>
    </head>
    <body>
        <p><button type="submit" id="set-time-button">Set local time for all candles</button></p>
        <p>Select a candle to control:</p>
        <div id="candle-button-list"></div>
        <p>Select palette colors:</p>
        <button type="button" id="reset-button" disabled="disabled" style="margin-right: 5px;">Reset</button><button type="button" id="reset-all-button">Reset all candles</button><br />
        <form id="set-palette-form" style="margin-top: 5px;">
            <input type="text" id="palette-color1" value="red" />
            <input type="text" id="palette-color2" value="orange" />
            <input type="text" id="palette-color3" value="yellow" />
            <br />
            <button type="submit" id="palette-submit-button" disabled="disabled" style="margin-right: 5px;">Set palette</button>
            <button type="submit" id="palette-submit-all-button">Set palette for all candles</button>
        </form>
    </body>
</html>
)INDEX_HTML_EOF";

void index_html_render(WebServer& server, size_t numCandles)
{
    char* responseBuf = new char[sizeof(INDEX_HTML_TEMPLATE) + 20];
    snprintf(responseBuf, sizeof(INDEX_HTML_TEMPLATE) + 20, INDEX_HTML_TEMPLATE, numCandles);
    server.send(200, "text/html", responseBuf);
    delete[] responseBuf;
}
