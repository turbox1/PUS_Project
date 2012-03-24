$(document).ready(function() {
    $('#send-link').click(function() {
        $.ajax({
            //url: 'cgi-bin/vscpc.cgi',
            url: 'test.json',
            success: function(data) {
                var a = $.parseJSON(data);
                $('#class').append(a.class);
                $('#type').append(a.type);
                $('#data').append(a.data);
            }
        });
        return false;
    });
});
