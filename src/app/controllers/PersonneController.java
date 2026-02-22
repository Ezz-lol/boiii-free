package app.controllers;

import app.entites.Personne;
import app.ihm.PersonneIHM;
import app.services.PersonneService;

public class PersonneController {
	
	PersonneService personneService;

	public PersonneController(PersonneService personneService) {
		this.personneService = personneService;
	}

	public void  init() {
		Personne p = PersonneIHM.saisir();
		this.personneService.calculerAge(p);
	}
	

}
