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
        interval: 1000000
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
            var deviceId = '#device-'+this.class+'-'+this.type+'-'+this.id;
            $(deviceId + ' .value').html(this.params.value);

            if (this.type == 6) {
                var unit;
                switch (parseInt(this.params.unit)) {
                    case 0: unit = "K";
                    case 1: unit = "C";
                    case 2: unit = "C";    
                }
                $(deviceId + ' .unit').html(unit);
            } 
            else if(this.type == 35) {
		        $('#humidity_progress').val(this.params.value);
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


var Thermometer = Backbone.Model.extend({
    
    defaults: {
        //class: null,
        //type: null,
        //id: null,
        no: 1,
        value: 20,
        unit: "&deg;C",
    },

    initialize: function() {
        console.log('sssssss');
    }
});

var Hygrometer = Backbone.Model.extend({
    
    defaults: {
        //class: null,
        //type: null,
        //id: null,
        no: 1,
        value: 20,
        unit: "%RH",
    },

    initialize: function() {
        console.log('sssssss');
    }
});

var thermometer1 = new Thermometer({value: 10, no: 1});
var thermometer2 = new Thermometer({value: 30, no: 2});

var hygrometer1 = new Hygrometer({value: 10, no: 1});
var hygrometer2 = new Hygrometer({value: 70, no: 2});

var ThermometerCollection = Backbone.Collection.extend({
    model: Thermometer
});

var HygrometerCollection = Backbone.Collection.extend({
    model: Hygrometer
});


var thermometerCollection = new ThermometerCollection([thermometer1, thermometer2]);

var hygrometerCollection = new HygrometerCollection([hygrometer1, hygrometer2]);

var ThermometerView = Backbone.View.extend({

    tagName: "li",
    template: _.template($("#thermometer-template").html()),

    initialize: function() {},

    render: function() {
      this.$el.html(this.template(this.model.toJSON()));
      return this;
    }

});

var HygrometerView = Backbone.View.extend({

    tagName: "li",
    template: _.template($("#hygrometer-template").html()),

    initialize: function() {},

    render: function() {
      this.$el.html(this.template(this.model.toJSON()));
      return this;
    }
});


var AppView = Backbone.View.extend({
   
    el: $("#ApplicationContainer"),
    
    initialize: function() {
        this.addAll();
    },

    render: function() {},

    addOne: function(model) {
        // Tutaj należy zdecydować co to jest za model. najlepiej utworzyć do tego osobny obiekt, 
        // który będzie za to odpowiedzialny

      var view = new ThermometerView({model: model});
      this.$("#DevicesList").append(view.render().el);
    },

    // Tymczasowe rozbicie na dwie metody
    addOne2: function(model) {
        // Tutaj należy zdecydować co to jest za model. najlepiej utworzyć do tego osobny obiekt, 
        // który będzie za to odpowiedzialny

      var view = new HygrometerView({model: model});
      this.$("#DevicesList").append(view.render().el);
    },

    addAll: function() {
      thermometerCollection.each(this.addOne);
      hygrometerCollection.each(this.addOne2);
    },
  });

var App = new AppView;

//var app = App;
//app.run(); 


