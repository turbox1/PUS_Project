

$(function() {
    $("#createRoom").click(function () {
		$("#DevicesTree").jstree("create",-1,"first","Podaj nazwę");
        //console.log($("#DevicesTree").jstree("get_json")); 
	});
    $("#DevicesTree")
	    .jstree({
		    core : { 
                "initially_open" : [ "aDevices" ]
            },
            "types" : {
			    "valid_children" : [ "root" ],
			    "types" : {
				    "root" : {
					    "icon" : { 
						    "image" : "/js/jstree/_docs/_drive.png" 
					    },
    					"valid_children" : [ "default" ],
	    				"max_depth" : 2,
		    			"hover_node" : false,
			    		"select_node" : function (node , check , e) {
                            console.info('Dodaje urządzenia z wybranego pokoju do szczegółowego widoku.');

                            $("#DevicesList").html('');
                            node.find('li').each(function(index) {
                                var dId = $(this).attr('id').substr(8).split("-");
                                //console.log(dId);
                                //$('#aDevices ul').html('');
                                DeviceFactory.update({
                                    "class": dId[0],
                                    "type": dId[1],
                                    "id": dId[2]
                                });
                            });
                            //return false;
                        }
				    },
    				"default" : {
	    				"valid_children" : [ "default" ]
                    }
                }
            },
            "html_data" : {
			    "data" : "<li id='aDevices' rel='root'><a href='#'>Dostępne urządzenia</a></li>"
	    	},
    		plugins : [ "themes", "html_data", "json_data", "types", "ui", "crrm", "dnd", "cookies"]
	});

    // models
    var Thermometer = Backbone.Model.extend({
    
        defaults: {
            class: null,
            type: null,
            id: null,
            did: null,
            value: 0,
            unit: "&deg;C",
        },

        initialize: function() {}
    });

    var Hygrometer = Backbone.Model.extend({
    
        defaults: {
            class: null,
            type: null,
            id: null,
            did: null,
            value: 0,
            unit: "%RH"
        },

        initialize: function() {}
    });

    // collections
    var ThermometerCollection = Backbone.Collection.extend({
        model: Thermometer
    });

    var HygrometerCollection = Backbone.Collection.extend({
        model: Hygrometer
    });


    var thermometerCollection = new ThermometerCollection();
    var hygrometerCollection = new HygrometerCollection();

    // views
    var ThermometerView = Backbone.View.extend({

        tagName: "li",
        template: _.template($("#thermometer-template").html()),

        initialize: function() {
            this.model.bind('change', this.render, this);
        },

        render: function() {
            this.$el.html(this.template(this.model.toJSON()));
            return this;
        }
    });

    var HygrometerView = Backbone.View.extend({

        tagName: "li",
        template: _.template($("#hygrometer-template").html()),

        initialize: function() {
            this.model.bind('change', this.render, this);
        },

        render: function() {
            this.$el.html(this.template(this.model.toJSON()));
            return this;
        }
    });

    var DeviceFactory = (function() {  
        var _thermometer = function(data) {

            var unit;
            switch (parseInt(data.params.unit)) {
                case 0: unit = "K";
                case 1: unit = "&deg;C";
                case 2: unit = "&deg;C";
                default: unit = "&deg;C"; 
            }
            
            if (thermometerCollection.get(data.id)) {
                thermometerCollection.get(data.id).set({value: data.params.value});
                console.log(data.params.value);
            } 
            else {
                var model = new Thermometer({
                    class:  data.class,
                    type:   data.type,
                    id:     data.id,
                    value:  data.params.value,
                    unit:   unit
                });
                thermometerCollection.push(model);
                //var view = new ThermometerView({model: model});
                //$("#DevicesList").append(view.render().el);
            }
        };

        var _hygrometer = function(data) {

            if (hygrometerCollection.get(data.id)) {
                hygrometerCollection.get(data.id).set({value: data.params.value});
            }
            else {
                var model = new Hygrometer({
                    class:  data.class,
                    type:   data.type,
                    id:     data.id,
                    value:  data.params.value
                })
            
                hygrometerCollection.push(model);
                //var view = new HygrometerView({model: model});
                //$("#DevicesList").append(view.render().el);
	        }
        }; 

        var _thermometer2 = function(data) {

            var model = thermometerCollection.get(data.id)
            var view = new ThermometerView({model: model});
            $("#DevicesList").append(view.render().el);
        };


        return {
            make: function(data) {
                if (data.class == 10) {
                    if (data.type == 6) {
                        _thermometer(data);
                    }
                    else if (data.type == 35) {
                        _hygrometer(data);
                    }
                }
            },
            update: function(data) {
                if (data.type == 6) {
                    _thermometer2(data);
                }
                else if (data.type == 35) {
                    _hygrometer2(data);
                }
            }
        };
    }());

     var TreeFactory = (function() {  
       
        return {
            make: function(data) {
                if (data.class == 10) {
                    if (data.type == 6) {
                        $("#DevicesTree").jstree("create",
                            "#aDevices",
                            false,
                            {
                               "attr": { "id" : "tDevice-"+data.class+"-"+data.type+"-"+data.id  },
                               "data": "Termometr"+data.id
                            },
                            false, true);

                    }
                    //else if (data.type == 35) {
                        //
                    //}
                }
            }
        };
    }());


    var MockDeamon = (function() {
    
        var _options = {
            interval: 5000,
            a: -1,
            data: [
                [
                    {"class":10, "type":6, "id":21, params: {"unit":0, "value":"0"}},
                    {"class":10, "type":35, "id":3, params: {"value":"14"}}
                ],
                [
                    {"class":10, "type":6, "id":22, params: {"unit":0, "value":"10"}},
                    {"class":10, "type":35, "id":3, params: {"value":"84"}},
                    {"class":10, "type":6,  "id":2, params: {"value":"23"}},
                    {"class":10, "type":35, "id":4, params: {"value":"33"}},
                ],
                [
                    {"class":10, "type":6, "id":22, params: {"unit":0, "value":"34"}},
                    {"class":10, "type":35, "id":3, params: {"value":"72"}},
                    {"class":10, "type":6,  "id":2, params: {"value":"26"}},
                    {"class":10, "type":35, "id":4, params: {"value":"34"}},
                ],
                [
                    {"class":10, "type":6, "id":22, params: {"unit":0, "value":"37"}},
                    {"class":10, "type":35, "id":3, params: {"value":"64"}},
                    {"class":10, "type":6,  "id":2, params: {"value":"22"}},
                    {"class":10, "type":35, "id":4, params: {"value":"35"}},
                ],
                [
                    {"class":10, "type":6, "id":22, params: {"unit":0, "value":"39"}},
                    {"class":10, "type":35, "id":3, params: {"value":"54"}},
                    {"class":10, "type":6,  "id":2, params: {"value":"20"}},
                    {"class":10, "type":35, "id":4, params: {"value":"36"}},
                ],
                [
                    {"class":10, "type":6, "id":22, params: {"unit":0, "value":"43"}},
                    {"class":10, "type":35, "id":3, params: {"value":"47"}},
                    {"class":10, "type":6,  "id":2, params: {"value":"18"}},
                    {"class":10, "type":35, "id":4, params: {"value":"38"}},
                ],
                [
                    {"class":10, "type":6, "id":22, params: {"unit":0, "value":"46"}},
                    {"class":10, "type":35, "id":3, params: {"value":"40"}},
                    {"class":10, "type":6,  "id":2, params: {"value":"14"}},
                    {"class":10, "type":35, "id":4, params: {"value":"39"}},
                ],
                [
                    {"class":10, "type":6, "id":22, params: {"unit":0, "value":"52"}},
                    {"class":10, "type":35, "id":3, params: {"value":"23"}},
                    {"class":10, "type":6,  "id":2, params: {"value":"11"}},
                    {"class":10, "type":35, "id":4, params: {"value":"41"}},
                ],
            ],
        };

        var run = function() {
            var self = this;
            (function worker() {
         
                setTimeout(worker, self.getOptions.interval);
         
                if (self.getOptions.a > -1) {
                    App.updateDevices(self.getOptions.data[self.getOptions.a]);
                }
                
                if (self.getOptions.a ==  7) {
                    self.getOptions.a = 0;
                }
                self.getOptions.a++;
            })();
        }

        return {
            run: run,
            getOptions: _options
        }
    }());

    var Deamon = (function() {
    
        var _options = {
            url     : "server.php",
            data    : "class=15&type=0",
            interval: 5000,
            a       : 0
        };

        var run = function() {
            var self = this;
            (function worker() {
                $.ajax({
                    url: self.getOptions.url,
                    dataType: 'json',
                    data: self.getOptions.data+"&i="+self.getOptions.a,
                    success: function(response) {
                        if (self.getOptions.a ==  2) {
                            self.getOptions.a = -1;
                        }
                        self.getOptions.a++;

                        App.updateDevices(response.data);
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
    }());

    //var Deamon = MockDeamon;


    var AppView = Backbone.View.extend({
   
        el: $("#ApplicationContainer"),
    
        initialize: function() {
            Deamon.run();
        },

        updateDevices: function(data) {
            //this.$("#DevicesList").html('');
            //thermometerCollection.reset();
            //hygrometerCollection.reset();
            $('#aDevices ul').html('');
            $(data).each(function() {
                DeviceFactory.make(this);
                TreeFactory.make(this);
            });
        },

        render: function() {}
    });


    
    

    // finally start app
    var App = new AppView;

    

}); //end of application


