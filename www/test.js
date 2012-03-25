$(document).ready(function() {
    $('#send-link').click(function() {
        $.ajax({
            url: 'cgi-bin/vscpc.cgi',
           // url: 'test.json',
            dataType: 'json',
            data: "class=20&type=10&data=222",
            success: function(data) {
                var a = data;
                $('#class').append(a.class);
                $('#type').append(a.type);
                $('#data').append(a.data);
            }
        });
        return false;
    });
});


var App = {
    
    options: {},

    run: function() {
        var self = this;
        self.Deamon().run();
    }

};

App.Deamon = function() {
    
    var _options = {
        url     : "cgi-bin/vscpc.cgi",
        data    : "class=15&type=0",
        interval: 10000
    };

    var run = function() {
        var self = this;
        (function worker() {
                $.ajax({
                    url: self.getOptions.url,
                    dataType: 'json',
                    data: self.getOptions.data,
                    success: function(response) {
                        App.Builder.update(response);
                    },
                    complete: function() {
                        setTimeout(worker, self.getOptions.interval);
                    }
                });
        })();
    }

    return {
        run: run,
        getOptions: _options
    }
};

App.Builder = (function() { 

    var _devicesDatabase;
    
    function _addData(data) {
        _devicesDatabase = data;
    };

    function _refreshAll() {
        $(_devicesDatabase).each(function() {
            if (this.type == 6) {
                $('#class').html(this.class);
                $('#type').html(this.type);
                $('#id').html(this.id);
                $('#value').html(this.params.value);
                var unit;
                switch (parseInt(this.params.unit)) {
                    case 0: unit = "K";
                    case 1: unit = "C";
                    case 2: unit = "F";    
                }
                $('#unit').html(unit);
            }
        });
    };

    return {
        update: function(data) {
            _addData(data);
            _refreshAll(); 
        }
    }

}());


var app = App;
app.run(); 


