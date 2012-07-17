

$(function() {
    
    // główny obiekt aplikacji
    // options - konfiguracja
    var App = {
        options: {
            url: 'server.php',
            config_url: 'config.php'
        },
        view: {}
    }
    
    // Binduje event dodawania nowego pokoju do drzewka  
    $("#createRoom").click(function (e) {
        e.preventDefault();
		$("#DevicesTree").jstree("create",-1,"first","Podaj nazwę");
	});

    // Binduje event usuwania elementu z drzewka
    $("#removeRoom").click(function (e) {
        e.preventDefault();
		var selected = $("#DevicesTree").jstree("get_selected");
       
        if (selected.filter('#aDevices').length) {
            return false;    
        }
        $('#DevicesTree').jstree('remove', selected);
	});

    // Tworze obiek drzewka do elementu #DevicesTree
    $("#DevicesTree")
        // Bindowanie eventu załadowania drzewka, dodajemy standardowy pokój "Dostępne urządzenia"
        .bind("loaded.jstree", function() {
            $('#DevicesTree').html('<ul><li id="aDevices" rel="root" class="jstree-open jstree-last"><ins class="jstree-icon">&nbsp;</ins><a href="#"><ins class="jstree-icon">&nbsp;</ins>Dostępne urządzenia</a><ul>');
            // Jeśli było zapisane w cookie to ładujemy z cookie
            if ($.cookie('devicesTree')) {
                 $('#DevicesTree').html($.cookie('devicesTree'));
            }
        })
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
                            // Po zaznaczeniu pokoju pokazujemy liste urządzeń z prawej strony
                            $("#DevicesList").html('');
                            node.find('li').each(function(index) {
                                var dId = $(this).attr('id').substr(8).split("-");
                                DeviceFactory.update({
                                    "class": dId[0],
                                    "type": dId[1],
                                    "id": dId[2]
                                });
                            });
                        }
				    },
    				"default" : {
	    				"valid_children" : [ "default" ]
                    }
                }
            },
            "crrm" : { 
			    "move" : {
			    	"check_move" : function (m) {
                        // sprawdzamy czy mozemy przenieść element do danego pokoju,
                        // w jednym pokoju moze byc jedno takie samo urządzenie
                        var deviceToMove = "#"+$(m.o).attr('id');
                        if ($(m.np).children('ul').find(deviceToMove).not(m.o).length) {
                            return false;
                        }
                        return true;
			    	}
	    		}
    		},
    		plugins : [ "themes", "html_data", "json_data", "types", "ui", "crrm", "dnd"]
	});

    // modele
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

    var Button = Backbone.Model.extend({
        dafaults: {
            class: null,
            type: null,
            id: null,
            did: null,
            value: 0,
            rep: 0
        },

        initialize: function() {},

        send: function() {
            var status = (this.get('type') == 4) ? 0 : 1;
            ButtonSender(this.get('class'), this.get('id'), status, this.get('rep'));
        }
    });

    // Wysyłanie stanu przycisku
    var ButtonSender = function(classVar, id, statusVar, repeat) {
        $.ajax({
            url: App.options.url,
            dataType: 'json',
            data: "class="+classVar+"&type=1&id="+id+"&value="+statusVar+"&repeat="+repeat,
            success: function(response) {
                App.view.updateDevices(response.data);
            }
        });
    };

    var Configuration = Backbone.Model.extend({
    
        defaults: {
            id: null,
            name: null,
            tree: {}
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

    var ButtonCollection = Backbone.Collection.extend({
        model: Button
    });

    var ConfigurationCollection = Backbone.Collection.extend({
        model: Configuration,
        url : App.options.config_url,
        parse : function(response) {
            return response;
        }
    });

    // inicjalizacja kolekcji    
    var thermometerCollection = new ThermometerCollection();
    var hygrometerCollection = new HygrometerCollection();
    var buttonCollection = new ButtonCollection();
    var configurationCollection = new ConfigurationCollection();

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

    var ButtonView = Backbone.View.extend({
        
        tagName: "li",
        template: _.template($('#button-template').html()),

        events: {
            'click .save-button': 'saveClick'
        },


        initialize: function() {
            this.model.bind('change', this.render, this);
        },

        render: function() {
            var model = this.model.toJSON();
            var modelObj = this.model;
            model.type = (model.type == 3) ? "Włączony" : "Wyłączony";
            this.$el.html(this.template(model));
            // Inicjalizacja pokrętła
            $(".knob").val(modelObj.get('rep')).knob({
                'release':function(e) {
                    modelObj.set('rep', e);
                    ButtonSender(modelObj.get('class'), modelObj.get('id'), modelObj.get('status'), e);
                }
            });
            return this;
        },

        saveClick: function() {
            this.model.send();
        }
    });

    var ConfigurationView = Backbone.View.extend({

        tagName: "option",

        initialize: function() {
            _.bindAll(this, 'render');
        },

        render: function() {
            $(this.el).attr('value', this.model.get('id')).html(this.model.get('name'));
            return this;
        }
    });


    var ConfigurationsView = Backbone.View.extend({
        el: $("#configurationBox"),

        events: {
            "change #confListSelect": "changeSelected"
        },
        
        initialize: function(){
            _.bindAll(this, 'addOne', 'addAll');
            this.collection.bind('reset', this.addAll);            
        },        
        addOne: function(model) {
            var configurationView = new ConfigurationView({ model: model });
            this.configurationsViews.push(configurationView);
            $('#confListSelect').append(configurationView.render().el);
        },        
        addAll: function() {
            _.each(this.configurationsViews, function(configurationView) { configurationView.remove(); });
            this.configurationsViews = [];
            this.collection.each(this.addOne);
            if (this.selectedId) {
                $(this.el).val(this.selectedId);
            }
        },
        changeSelected: function() {
            var savedConf = this.collection.get($('#confListSelect').val()).get('tree');
            if (savedConf) {
                $('#DevicesTree').html(savedConf);
            }
            else {
                $('#DevicesTree').html('<ul><li id="aDevices" rel="root" class="jstree-open jstree-last"><ins class="jstree-icon">&nbsp;</ins><a href="#"><ins class="jstree-icon">&nbsp;</ins>Dostępne urządzenia</a><ul>');
            }
        }
    });

    new ConfigurationsView({collection: configurationCollection});
    configurationCollection.fetch();

    // Fabryka urządzeń
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
                });
            
                hygrometerCollection.push(model);
	        }
        };

        var _button = function(data) {

            if (buttonCollection.get(data.id)) {
                buttonCollection.get(data.id).set({type: data.type});
            }
            else {
                var model = new Button({
                    class:  data.class,
                    type:   data.type,
                    id:     data.id,
                    rep:    0
                });
                $(".knob").knob({
                    'release':function(e) {
                        // po zwolnieniu pokrętła wysyłamy stan urządzenia
                        model.set('rep', e);
                        ButtonSender(model.get('class'), model.get('id'), model.get('status'), 0);
                    }
                });
            
                buttonCollection.push(model);
	        }
        };  

        var _thermometer2 = function(data) {
            var model = thermometerCollection.get(data.id)
            var view = new ThermometerView({model: model});
            $("#DevicesList").append(view.render().el);
        };

        var _hygrometer2 = function(data) {
            var model = hygrometerCollection.get(data.id)
            var view = new HygrometerView({model: model});
            $("#DevicesList").append(view.render().el);
        };

        var _button2 = function(data) {
            var model = buttonCollection.get(data.id);
            var view = new ButtonView({model: model});
            $("#DevicesList").append(view.render().el);
            $(".knob").knob({
                'release':function(e) {
                    model.set('rep', e);
                    ButtonSender(model.get('class'), model.get('id'), model.get('status'), e);
                }
            });
        };

        // Udostępniamy publicznie metody
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
                else if (data.class == 20) {
                    if (data.type == 3 || data.type == 4) {
                        _button(data);
                    }
                }
            },
            update: function(data) {
                if (data.class == 10) {
                    if (data.type == 6) {
                        _thermometer2(data);
                    }
                    else if (data.type == 35) {
                        _hygrometer2(data);
                    }
                }
                else if (data.class == 20) {
                    if (data.type == 3 || data.type == 4) {
                        _button2(data);
                    }
                }                
            }
        };
    }());

    // Fabryka dodawania urzadzen do drzewka
    var TreeFactory = (function() {  
       
        return {
            make: function(data) {
                if (data.class == 10) {
                    // termometr 
                    if (data.type == 6) {
                        $("#DevicesTree").jstree("create",
                            "#aDevices",
                            false,
                            {
                               "attr": { "id" : "tDevice-"+data.class+"-"+data.type+"-"+data.id },
                               "data": "Termometr"+data.id
                            },
                            false, true);
                    }
                    // higrometr
                    else if (data.type == 35) {
                        $("#DevicesTree").jstree("create",
                            "#aDevices",
                            false,
                            {
                               "attr": { "id" : "tDevice-"+data.class+"-"+data.type+"-"+data.id },
                               "data": "Hygrometr"+data.id
                            },
                            false, true);
                    }
                }
                else if (data.class == 20) {
                    // przycisk
                    if (data.type == 3 || data.type == 4) {
                        $("#DevicesTree").jstree("create",
                            "#aDevices",
                            false,
                            {
                               "attr": { "id" : "tDevice-"+data.class+"-"+data.type+"-"+data.id },
                               "data": "Przycisk"+data.id
                            },
                            false, true);
                    } 
                }
            }
        };
    }());

    // Przykładowy deamon udający urządzenia
    var MockDeamon = (function() {
    
        var _options = {
            interval: 5000000,
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

    // Deamon autoaktualizacji stanu urządzeń
    var Deamon = (function() {
    
        var _options = {
//            url     : "server.php",
            data    : "class=15&type=0",
            interval: 5000,
            a       : 0
        };

        var run = function() {
            var self = this;
            // worker uruchamiany z interwałem, pobiera stan urządzeń
            (function worker() {
                $.ajax({
                    url: App.options.url,
                    dataType: 'json',
                    data: self.getOptions.data+"&i="+self.getOptions.a,
                    success: function(response) {
                        if (self.getOptions.a ==  2) {
                            self.getOptions.a = -1;
                        }
                        self.getOptions.a++;
                        // aktualizacja stanu urządzeń na widoku
                        App.view.updateDevices(response.data);
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
            // startujemy deamona
            Deamon.run();
            // przy zamykaniu strony zapisujemy stan drzewka do cookie
            $(window).unload(function () {
                $.cookie('devicesTree', $("#DevicesTree").html(), { expires: 365 });                
            });
        },

        updateDevices: function(data) {
            $('#aDevices ul').html('');
            // przekazujemy pokolei urzadzenia do fabryki do utworzenia
            $(data).each(function() {
                DeviceFactory.make(this);
                TreeFactory.make(this);
            });
        },

        /*getConfigurationList: function() {
             $.ajax({
                url: App.options.config_url,
                dataType: 'json',
                success: function(response) {
                    configurationCollection.add(response);
                    configurationCollection.each(function(i) {
                        var view = new ConfigurationView({model: i});
                        $("#configListSelect").append(view.render().el);
                    });
                }
            });
        },*/

        render: function() {}
    });


    
    

    // startujemy aplikacje
    App.view = new AppView;

    

}); //koniec aplikacji


