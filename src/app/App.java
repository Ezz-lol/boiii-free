package app;

import app.controllers.PersonneController;
import app.services.PersonneService;

public class App {

	public static void main(String[] args) {
		
		PersonneController pc = new PersonneController(new PersonneService());
		pc.init();
		
	}
}
